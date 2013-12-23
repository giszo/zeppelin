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

    Config cfg;
    Json::Reader reader;

    if (!reader.parse(f, cfg.m_raw))
	throw ConfigException("unable to parse config");

    Json::Value& root = cfg.m_raw;

    // plugins section
    if (!root.isMember("plugins"))
	throw ConfigException("no plugins section");

    Json::Value plugins = root["plugins"];
    Json::Value pluginList = plugins["list"];

    cfg.m_plugin.m_root = plugins["root"].asString();

    for (Json::Value::ArrayIndex i = 0; i < pluginList.size(); ++i)
	cfg.m_plugin.m_list.push_back(pluginList[i].asString());

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
