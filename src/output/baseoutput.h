#ifndef OUTPUT_BASEOUTPUT_H_INCLUDED
#define OUTPUT_BASEOUTPUT_H_INCLUDED

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
	virtual ~BaseOutput()
	{}

	virtual int getRate() = 0;
	virtual int getChannels() = 0;

	/// returns the number of available space for free samples on the device
	virtual int getFreeSize() = 0;

	virtual void setup(int rate, int channels) = 0;

	virtual void drop() = 0;

	virtual void write(const int16_t* samples, size_t count) = 0;
};

}

#endif
