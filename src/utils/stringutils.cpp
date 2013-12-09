#include "stringutils.h"

#include <sstream>

using utils::StringUtils;

// =====================================================================================================================
bool StringUtils::endsWith(const std::string& s, const std::string& ending)
{
    if (s.length() < ending.length())
	return false;

    return s.compare(s.length() - ending.length(), ending.length(), ending) == 0;
}

// =====================================================================================================================
int StringUtils::toInt(const std::string& s)
{
	int result = 0;
	std::stringstream(s) >> result;
	return result;
}