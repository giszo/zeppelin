#include <output/alsa.h>
#include <codec/mp3.h>
#include <library/musiclibrary.h>
#include <library/sqlitestorage.h>
#include <player/controller.h>
#include <config/parser.h>
#include <plugin/pluginmanager.h>
#include <utils/signalhandler.h>
#include <utils/pidfile.h>

#include <boost/program_options.hpp>

#include <iostream>

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

    // perform global initialization
    mpg123_init();

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

    std::shared_ptr<zeppelin::library::MusicLibrary> lib =
	std::make_shared<library::MusicLibraryImpl>(storage, config.m_library);

    // create the main part of our wonderful player :)
    std::shared_ptr<zeppelin::player::Controller> ctrl = std::make_shared<player::ControllerImpl>(config);

    // initialize the plugin manager
    plugin::PluginManagerImpl pm(lib, ctrl);
    pm.loadAll(config);

    // start the main loop of the player
    std::static_pointer_cast<player::ControllerImpl>(ctrl)->start();

    // run the signal handler
    signalHandler.run();

    mpg123_exit();

    return 0;
}
