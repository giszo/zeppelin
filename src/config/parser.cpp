#include "parser.h"

#include <jsonrpc/json/reader.h>

#include <fstream>

using config::Parser;

// =====================================================================================================================
Parser::Parser(const std::string& file)
    : m_file(file)
{
}

// =====================================================================================================================
config::Config Parser::parse() const
{
    std::ifstream f(m_file);

    if (!f.is_open())
	throw ConfigException("unable to open file");

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(f, root))
	throw ConfigException("unable to parse config");

    Config cfg;

    // rpc section
    if (!root.isMember("rpc"))
	throw ConfigException("no RPC section");

    Json::Value rpc = root["rpc"];

    if (!rpc.isMember("address") || !rpc.isMember("port"))
	throw ConfigException("RPC section is incomplete");

    cfg.m_rpc.m_address = rpc["address"].asString();
    cfg.m_rpc.m_port = rpc["port"].asInt();

    // library section
    if (!root.isMember("library"))
	throw ConfigException("no library section");

    Json::Value library = root["library"];

    if (!library.isMember("root"))
	throw ConfigException("library section is incomplete");

    Json::Value libRoots = library["root"];

    for (Json::Value::ArrayIndex i = 0; i < libRoots.size(); ++i)
	cfg.m_library.m_root.push_back(libRoots[i].asString());

    return cfg;
}
