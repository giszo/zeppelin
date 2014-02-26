/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CONFIG_PARSER_H_INCLUDED
#define CONFIG_PARSER_H_INCLUDED

#include "config.h"

#include <string>
#include <stdexcept>

namespace config
{

class ConfigException : public std::runtime_error
{
    public:
	ConfigException(const std::string& error)
	    : runtime_error(error)
	{}
};

class Parser
{
    public:
	Parser(const std::string& file);

	Config parse() const;

    private:
	void parsePlugins(const Json::Value& config, Plugins& plugins) const;
	void parseLibrary(const Json::Value& config, Library& library) const;

    private:
	std::string m_file;
};

}

#endif
