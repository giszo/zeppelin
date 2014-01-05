#include "server.h"

// =====================================================================================================================
extern "C"
plugin::Plugin* plugin_create(const std::shared_ptr<library::MusicLibrary>& library,
			      const std::shared_ptr<player::Controller>& ctrl)
{
    return new Server();
}
