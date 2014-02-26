/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CONFIG_CONFIG_H_INCLUDED
#define CONFIG_CONFIG_H_INCLUDED

#include <jsoncpp/json/value.h>

#include <string>
#include <vector>
#include <unordered_map>

namespace config
{

struct Plugins
{
    std::string m_root;
    std::unordered_map<std::string, Json::Value> m_available;
    std::vector<std::string> m_enabled;
};

struct Library
{
    std::vector<std::string> m_roots;
    std::string m_database;
};

struct Config
{
    Plugins m_plugins;
    Library m_library;

    Json::Value m_raw;
};

}

#endif
