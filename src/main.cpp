#include <codec/codecmanager.h>
#include <library/musiclibrary.h>
#include <library/sqlitestorage.h>
#include <player/controller.h>
#include <config/parser.h>
#include <plugin/pluginmanager.h>
#include <utils/signalhandler.h>
#include <utils/pidfile.h>

#include <boost/program_options.hpp>

#include <iostream>

#ifdef HAVE_MP3
#include <mpg123.h>
#include <codec/mp3.h>
#endif

#ifdef HAVE_FLAC
#include <codec/flac.h>
#endif

#ifdef HAVE_OGG
#include <codec/vorbis.h>
#endif

#ifdef HAVE_WAVPACK
#include <codec/wavpack.h>
#endif

#ifdef HAVE_MONKEYSAUDIO
#include <codec/mac.h>
#endif

#ifdef HAVE_ALSA
#include <output/alsa.h>
#endif

#ifdef HAVE_PULSEAUDIO
#include <output/pulseaudio.h>
#endif

static bool s_daemonize = false;
static std::string s_config;
static std::string s_pidFile;

static std::unique_ptr<utils::PidFile> s_pf;

// =====================================================================================================================
static bool parseArgs(int argc, char** argv)
{
    boost::program_options::options_description desc;
    desc.add_options()
	("help,h", "output help message")
	("config,c", boost::program_options::value<std::string>(), "configuration file")
	("pidfile,p", boost::program_options::value<std::string>(), "pid file")
	("daemonize,d", "daemonize the process")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    // display help if it was requested
    if (vm.count("help"))
    {
	std::cout << desc << std::endl;
	return false;
    }

    // make sure we have the path of the config from command line
    if (!vm.count("config"))
    {
	std::cout << "missing configuration parameter" << std::endl;
	return false;
    }

    s_config = vm["config"].as<std::string>();
    s_daemonize = vm.count("daemonize");

    if (vm.count("pidfile"))
	s_pidFile = vm["pidfile"].as<std::string>();

    return true;
}

// =====================================================================================================================
int main(int argc, char** argv)
{
    if (!parseArgs(argc, argv))
	return 1;

    if (s_daemonize)
    {
	if (daemon(0, 1) != 0)
	{
	    std::cerr << "unable to daemonize!" << std::endl;
	    return 1;
	}

	// create the PID file... the destructor of the object will delete it
	if (!s_pidFile.empty())
	    s_pf = utils::PidFile::create(s_pidFile);
    }

    // load configuration
    config::Config config;
    config::Parser parser(s_config);

    try
    {
	config = parser.parse();
    }
    catch (const config::ConfigException& e)
    {
	std::cerr << "Unable to load configuration: " << e.what() << std::endl;
	return 1;
    }

    // create the signal handler before anything else to setup signal masking
    utils::SignalHandler signalHandler;

    // open the music library
    library::SqliteStorage storage;

    try
    {
	storage.open(config.m_library);
    }
    catch (const zeppelin::library::StorageException& e)
    {
	std::cerr << "Unable to open music library storage: " << e.what() << std::endl;
	return 1;
    }

    codec::CodecManager codecManager;

#ifdef HAVE_MP3
    mpg123_init();
    codecManager.registerCodec("mp3",  [](const std::string& file) { return std::make_shared<codec::Mp3>(file); });
#endif
#ifdef HAVE_FLAC
    codecManager.registerCodec("flac", [](const std::string& file) { return std::make_shared<codec::Flac>(file); });
#endif
#ifdef HAVE_OGG
    codecManager.registerCodec("ogg",  [](const std::string& file) { return std::make_shared<codec::Vorbis>(file); });
#endif
#ifdef HAVE_WAVPACK
    codecManager.registerCodec("wv",   [](const std::string& file) { return std::make_shared<codec::WavPack>(file); });
#endif
#ifdef HAVE_MONKEYSAUDIO
    codecManager.registerCodec("ape",  [](const std::string& file) { return std::make_shared<codec::Mac>(file); });
#endif

    std::shared_ptr<zeppelin::library::MusicLibrary> lib =
	std::make_shared<library::MusicLibraryImpl>(codecManager, storage, config.m_library);

    // prepare the audio output
    std::shared_ptr<output::BaseOutput> output;

#if defined(HAVE_ALSA)
    output = std::make_shared<output::AlsaOutput>(config);
#elif defined(HAVE_PULSEAUDIO)
    output = std::make_shared<output::PulseAudio>(config);
#else
#error "There is no configured audio output driver!"
#endif

    output->setup(44100, 2);

    player::Fifo fifo(4 * 1024);
    player::Format fmt = output->getFormat();

    std::shared_ptr<player::Decoder> decoder(new player::Decoder(fmt.sizeOfSeconds(10 /* 10 seconds of samples */), fmt, fifo, config));
    decoder->start();

    std::shared_ptr<player::Player> player(new player::Player(output, fifo, config));
    player->start();

    fifo.setNotifyCallback(fmt.sizeOfSeconds(5 /* 5 second limit */), std::bind(&player::Decoder::notify, decoder.get()));

    // create the main part of our wonderful player :)
    std::shared_ptr<zeppelin::player::Controller> ctrl = player::ControllerImpl::create(codecManager, decoder, player, config);

    // initialize the plugin manager
    plugin::PluginManagerImpl pm(lib, ctrl);
    pm.loadAll(config);

    // start the main loop of the player
    std::static_pointer_cast<player::ControllerImpl>(ctrl)->start();

    // run the signal handler
    signalHandler.run();

#ifdef HAVE_MP3
    mpg123_exit();
#endif

    return 0;
}
