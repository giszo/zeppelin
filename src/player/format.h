#ifndef PLAYER_FORMAT_H_INCLUDED
#define PLAYER_FORMAT_H_INCLUDED

#include <stdexcept>

#include <stddef.h>

namespace player
{

class FormatException : public std::runtime_error
{
    public:
	FormatException(const std::string& error) :
	    runtime_error(error)
	{}
};

class Format
{
    public:
	Format(int rate, int channels);

	int getRate() const;
	int getChannels() const;

	// returns the size (in bytes) of samples for the given amount of time
	size_t sizeOfSeconds(unsigned secs) const;
	// returns the size (in bytes) of the given amount of samples according to the format
	size_t sizeOfSamples(size_t count) const;

	// returns the number of samples found in the given size
	size_t numOfSamples(size_t size) const;

    private:
	int m_rate;
	int m_channels;
};

}

#endif
