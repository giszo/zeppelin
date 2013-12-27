#include "volume.h"

#include <iostream>
#include <cmath>

using filter::Volume;

// =====================================================================================================================
Volume::Volume(const config::Config& config)
    : BaseFilter(config, "volume"),
      m_level(1.0)
{
}

// =====================================================================================================================
void Volume::setLevel(float level)
{
    m_level = (expf(level) - 1) / (M_E - 1);
}

// =====================================================================================================================
void Volume::init()
{
}

// =====================================================================================================================
void Volume::run(float*& samples, size_t& count, const player::Format& format)
{
    float level = m_level;

    for (size_t i = 0; i < count * format.getChannels(); ++i)
	samples[i] *= level;
}
