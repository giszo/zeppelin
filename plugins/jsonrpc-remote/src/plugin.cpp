#include "server.h"

#include <iostream>

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
	std::cout << "jsonrpc-remote: no RPC section in config" << std::endl;
	return NULL;
    }

    Json::Value rpc = root["rpc"];

    if (!rpc.isMember("address") || !rpc.isMember("port"))
    {
	std::cout << "jsonrpc-remote: no address and/or port" << std::endl;
	return NULL;
    }

    return new Server(rpc["port"].asInt(), library, ctrl);
}
