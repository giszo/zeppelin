#ifndef FILTER_BASEFILTER_H_INCLUDED
#define FILTER_BASEFILTER_H_INCLUDED

#include <stddef.h>

namespace filter
{

class BaseFilter
{
    public:
	virtual ~BaseFilter()
	{}

	virtual void run(float* samples, size_t count) = 0;
};

}

#endif
