/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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

    // make sure root is an object
    if (!root.isObject())
	throw ConfigException("root node is not an object");

    // plugins section
    if (root.isMember("plugins") && root["plugins"].isObject())
	parsePlugins(cfg.m_raw["plugins"], cfg.m_plugins);

    // library section
    if (!root.isMember("library") || !root["library"].isObject())
	throw ConfigException("no library section");

    parseLibrary(root["library"], cfg.m_library);

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

// =====================================================================================================================
void Parser::parseLibrary(const Json::Value& config, Library& library) const
{
    // roots
    if (!config.isMember("roots"))
	throw ConfigException("roots is not configured for library");

    const Json::Value& roots = config["roots"];

    for (Json::Value::ArrayIndex i = 0; i < roots.size(); ++i)
	library.m_roots.push_back(roots[i].asString());

    // database
    if (config.isMember("database") && config["database"].isString())
	library.m_database = config["database"].asString();
}
