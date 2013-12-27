#ifndef FILTER_BASEFILTER_H_INCLUDED
#define FILTER_BASEFILTER_H_INCLUDED

#include <player/format.h>

#include <stdexcept>

#include <stddef.h>

namespace filter
{

class FilterException : public std::runtime_error
{
    public:
	FilterException(const std::string& error) :
	    runtime_error(error)
	{}
};

class BaseFilter
{
    public:
	virtual ~BaseFilter()
	{}

	/**
	 * Initializes the filter.
	 * FitlterException may thrown in case of the filter initialization failed.
	 */
	virtual void init() = 0;

	virtual void run(float*& samples, size_t& count, const player::Format& format) = 0;
};

}

#endif
