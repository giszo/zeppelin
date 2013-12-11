#ifndef CONFIG_CONFIG_H_INCLUDED
#define CONFIG_CONFIG_H_INCLUDED

#include <string>
#include <vector>

namespace config
{

struct RPC
{
    std::string m_address;
    int m_port;
};

struct Library
{
    std::vector<std::string> m_root;
};

struct Config
{
    RPC m_rpc;
    Library m_library;
};

}

#endif
