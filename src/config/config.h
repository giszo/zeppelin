#ifndef CONFIG_CONFIG_H_INCLUDED
#define CONFIG_CONFIG_H_INCLUDED

#include <jsonrpc/json/value.h>

#include <string>
#include <vector>

namespace config
{

struct Plugin
{
    std::string m_root;
    std::vector<std::string> m_list;
};

struct Library
{
    std::vector<std::string> m_root;
};

struct Config
{
    Plugin m_plugin;
    Library m_library;

    Json::Value m_raw;
};

}

#endif
