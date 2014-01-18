#include <output/alsa.h>
#include <codec/mp3.h>
#include <library/musiclibrary.h>
#include <library/sqlitestorage.h>
#include <player/controller.h>
#include <config/parser.h>
#include <plugin/pluginmanager.h>
#include <utils/signalhandler.h>

#include <iostream>

// =====================================================================================================================
int main(int argc, char** argv)
{
    if (argc != 2)
    {
	std::cerr << "Config parameter missing!" << std::endl;
	return 1;
    }

    // perform global initialization
    mpg123_init();

    // load configuration
    config::Config config;
    config::Parser parser(argv[1]);

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
