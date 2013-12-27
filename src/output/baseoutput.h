#ifndef OUTPUT_BASEOUTPUT_H_INCLUDED
#define OUTPUT_BASEOUTPUT_H_INCLUDED

#include <config/config.h>
#include <player/format.h>

#include <stdexcept>

namespace output
{

class OutputException : public std::runtime_error
{
    public:
	OutputException(const std::string& error) :
	    runtime_error(error)
	{}
};

class BaseOutput
{
    public:
	BaseOutput(const config::Config& config, const std::string& name);

	virtual ~BaseOutput()
	{}

	// returns the format (sampling rate, channels, etc.) of the output
	virtual player::Format getFormat() const = 0;

	/// returns the number of available space for free samples on the device
	virtual int getFreeSize() = 0;

	virtual void setup(int rate, int channels) = 0;

	virtual void drop() = 0;

	virtual void write(const float* samples, size_t count) = 0;

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
