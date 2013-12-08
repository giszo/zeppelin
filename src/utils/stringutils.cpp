#include "stringutils.h"

using utils::StringUtils;

// =====================================================================================================================
bool StringUtils::endsWith(const std::string& s, const std::string& ending)
{
    if (s.length() < ending.length())
	return false;

    return s.compare(s.length() - ending.length(), ending.length(), ending) == 0;
}
