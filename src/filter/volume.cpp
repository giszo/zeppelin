#include "volume.h"

#include <iostream>
#include <cmath>

using filter::Volume;

// =====================================================================================================================
Volume::Volume()
    : m_level(1.0)
{
}

// =====================================================================================================================
void Volume::setLevel(float level)
{
    m_level = (expf(level) - 1) / (M_E - 1);
}

// =====================================================================================================================
void Volume::run(float* samples, size_t count)
{
    float level = m_level;

    for (size_t i = 0; i < count * 2; ++i)
	samples[i] *= level;
}
