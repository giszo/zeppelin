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
	std::string m_file;
};

}

#endif
