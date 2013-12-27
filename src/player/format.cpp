#include "format.h"

using player::Format;

// =====================================================================================================================
Format::Format(int rate, int channels)
    : m_rate(rate),
      m_channels(channels)
{
}

// =====================================================================================================================
int Format::getRate() const
{
    return m_rate;
}

// =====================================================================================================================
int Format::getChannels() const
{
    return m_channels;
}

// =====================================================================================================================
size_t Format::sizeOfSeconds(unsigned secs) const
{
    return sizeof(float) * m_rate * m_channels * secs;
}

// =====================================================================================================================
size_t Format::sizeOfSamples(size_t count) const
{
    return sizeof(float) /* we use float samples in the player */ * m_channels * count;
}

// =====================================================================================================================
size_t Format::numOfSamples(size_t size) const
{
    if (size % (sizeof(float) * m_channels) != 0)
	throw FormatException("size is not the multiple of samplesize * channels");

    return size / (sizeof(float) * m_channels);
}
