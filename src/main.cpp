#include <output/alsa.h>
#include <codec/mp3.h>
#include <library/musiclibrary.h>
#include <library/parsefiles.h>
#include <rpc/server.h>
#include <player/controller.h>

#include <iostream>

// =====================================================================================================================
int main(int argc, char** argv)
{
    // perform global initialization
    mpg123_init();

    // open the music library
    library::MusicLibrary lib;
    lib.open();

    // try to parse the metadate of new files
    lib.addWork(std::make_shared<library::ParseFiles>(lib));

    // create the main part of our wonderful player :)
    player::Controller ctrl;

    // start the RPC server
    rpc::Server server(lib, ctrl);
    server.StartListening();

    // run the main loop of the player
    ctrl.run();

    mpg123_exit();

    return 0;
}
