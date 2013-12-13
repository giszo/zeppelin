#ifndef UTILS_MAKESTRING_H_INCLUDED
#define UTILS_MAKESTRING_H_INCLUDED

#include <sstream>

namespace utils
{

class MakeString
{
    public:
	template<typename T>
	MakeString& operator<<(const T& t)
	{ m_stream << t; return *this; }

	operator std::string()
	{ return m_stream.str(); }

    private:
	std::ostringstream m_stream;
};

}

#endif
