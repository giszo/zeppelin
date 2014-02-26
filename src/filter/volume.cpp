/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "volume.h"

#include <cmath>

using filter::Volume;

// =====================================================================================================================
Volume::Volume(const config::Config& config)
    : BaseFilter(config, "volume"),
      m_linearLevel(100),
      m_level(1.0f)
{
}

// =====================================================================================================================
int Volume::getLevel() const
{
    return m_linearLevel;
}

// =====================================================================================================================
bool Volume::setLevel(int level)
{
    if (level < 0 || level > 100)
	return false;

    float l = level / 100.0f;

    m_linearLevel = level;
    m_level = (expf(l) - 1) / (M_E - 1);

    return true;
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
