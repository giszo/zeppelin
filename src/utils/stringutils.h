#ifndef UTILS_STRINGUTILS_H_INCLUDED
#define UTILS_STRINGUTILS_H_INCLUDED

#include <string>

namespace utils
{

class StringUtils
{
	public:
		static bool endsWith(const std::string& s, const std::string& ending);
		static int toInt(const std::string& s);
};

}

#endif
