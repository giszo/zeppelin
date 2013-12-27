#ifndef FILTER_BASEFILTER_H_INCLUDED
#define FILTER_BASEFILTER_H_INCLUDED

#include <player/format.h>

#include <stddef.h>

namespace filter
{

class BaseFilter
{
    public:
	virtual ~BaseFilter()
	{}

	virtual void run(float* samples, size_t count, const player::Format& format) = 0;
};

}

#endif
