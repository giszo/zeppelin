#include <output/alsa.h>
#include <codec/mp3.h>
#include <library/musiclibrary.h>
#include <library/parsefiles.h>
#include <library/sqlitestorage.h>
#include <rpc/server.h>
#include <player/controller.h>
#include <config/parser.h>

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

    // open the music library
    library::SqliteStorage storage;

    try
    {
	storage.open();
    }
    catch (const library::StorageException& e)
    {
	std::cerr << "Unable to open music library storage: " << e.what() << std::endl;
	return 1;
    }

    library::MusicLibrary lib(storage);

    // try to parse the metadate of new files
    lib.addWork(std::make_shared<library::ParseFiles>(lib));

    // create the main part of our wonderful player :)
    player::Controller ctrl;

    // start the RPC server
    rpc::Server server(lib, ctrl, config.m_rpc);
    server.StartListening();

    // run the main loop of the player
    ctrl.run();

    mpg123_exit();

    return 0;
}
