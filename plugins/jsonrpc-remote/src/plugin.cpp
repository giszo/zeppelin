#include "server.h"

#include <logger.h>

// =====================================================================================================================
extern "C"
plugin::Plugin* plugin_create(const config::Config& config,
			      const std::shared_ptr<library::MusicLibrary>& library,
			      const std::shared_ptr<player::Controller>& ctrl)
{
    // parse configuration
    const Json::Value& root = config.m_raw;

    if (!root.isMember("rpc"))
    {
	LOG("jsonrpc-remote: no RPC section in config");
	return NULL;
    }

    Json::Value rpc = root["rpc"];

    if (!rpc.isMember("address") || !rpc.isMember("port"))
    {
	LOG("jsonrpc-remote: no address and/or port");
	return NULL;
    }

    return new Server(rpc["port"].asInt(), library, ctrl);
}
