#include "output/alsa.h"
#include "codec/mp3.h"
#include "library/musiclibrary.h"
#include "rpc/server.h"
#include "player/player.h"

#include <iostream>

// =====================================================================================================================
int main(int argc, char** argv)
{
    // perform global initialization
    mpg123_init();

    // open the music library
    library::MusicLibrary lib;
    lib.open();

    // create the main part of our wonderful player :)
    player::Player player;

    // start the RPC server
    rpc::Server server(lib, player);
    server.StartListening();

    // run the main loop of the player
    player.run();

    mpg123_exit();

    return 0;
}
