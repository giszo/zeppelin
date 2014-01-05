#include "parser.h"

#include <jsoncpp/json/reader.h>

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
    if (root.isMember("plugins"))
	parsePlugins(cfg.m_raw["plugins"], cfg.m_plugins);

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

// =====================================================================================================================
void Parser::parsePlugins(const Json::Value& config, Plugins& plugins) const
{
    if (!config.isMember("root"))
	throw ConfigException("directory of plugins not configured");

    // root
    plugins.m_root = config["root"].asString();

    // available
    if (config.isMember("available"))
    {
	if (!config["available"].isObject())
	    throw ConfigException("available under plugins is not an object");

	const Json::Value& avail = config["available"];
	auto members = avail.getMemberNames();

	for (const auto& plugin : members)
	    plugins.m_available[plugin] = avail[plugin];
    }

    // enabled
    if (config.isMember("enabled"))
    {
	if (!config["enabled"].isArray())
	    throw ConfigException("enabled under plugins is not an array");

	const Json::Value& enabled = config["enabled"];

	for (Json::Value::ArrayIndex i = 0; i < enabled.size(); ++i)
	{
	    std::string plugin = enabled[i].asString();

	    if (plugins.m_available.find(plugin) == plugins.m_available.end())
		throw ConfigException("plugin '" + plugin + "' not found in the available section");

	    plugins.m_enabled.push_back(plugin);
	}
    }
}
