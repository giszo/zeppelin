#ifndef UTILS_STRINGUTILS_H_INCLUDED
#define UTILS_STRINGUTILS_H_INCLUDED

#include <string>
#include <stdexcept>

namespace utils
{

class NumberFormatException : public std::runtime_error
{
    public:
	NumberFormatException(const std::string& error)
	    : runtime_error(error)
	{}
};

class StringUtils
{
    public:
	static bool endsWith(const std::string& s, const std::string& ending);

	static int toInt(const std::string& s);
};

}

#endif
