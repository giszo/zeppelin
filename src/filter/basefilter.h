#ifndef FILTER_BASEFILTER_H_INCLUDED
#define FILTER_BASEFILTER_H_INCLUDED

#include <player/format.h>
#include <config/config.h>

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
	BaseFilter(const config::Config& config, const std::string& name);

	virtual ~BaseFilter()
	{}

	/**
	 * Initializes the filter.
	 * FitlterException may thrown in case of the filter initialization failed.
	 */
	virtual void init() = 0;

	virtual void run(float*& samples, size_t& count, const player::Format& format) = 0;

    protected:
	// returns true in case of configuratio was set for this output
	bool hasConfig() const;
	/**
	 * Returns the configuration structure of the output.
	 * DO NOT call it without checking whether configuration is set with hasConfig()!
	 */
	const Json::Value& getConfig() const;

    private:
	const Json::Value* m_config;
};

}

#endif
