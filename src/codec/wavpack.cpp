/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "wavpack.h"

#include <zeppelin/logger.h>

using codec::WavPack;

// =====================================================================================================================
WavPack::WavPack(const std::string& file)
    : BaseCodec(file),
      m_context(NULL),
      m_floatMode(false),
      m_rate(0),
      m_channels(0),
      m_scale(0)
{
}

// =====================================================================================================================
WavPack::~WavPack()
{
    if (m_context)
	WavpackCloseFile(m_context);
}

// =====================================================================================================================
void WavPack::open()
{
    char error[100];

    m_context = WavpackOpenFileInput(m_file.c_str(), error, OPEN_WVC | OPEN_2CH_MAX /* whew ... */ | OPEN_NORMALIZE, 0);

    if (!m_context)
	throw CodecException("unable to open file");

    m_rate = WavpackGetSampleRate(m_context);
    m_channels = WavpackGetReducedChannels(m_context); // use GetNumChannels() once we support more than 2 ...
    m_floatMode = (WavpackGetMode(m_context) & MODE_FLOAT) != 0;

    if (m_channels != 2)
    {
	LOG("wavpack: unsupported channels: " << m_channels);
	throw CodecException("unsupported channels");
    }

    LOG("wavpack: using " << (m_floatMode ? "float" : "integer") << " mode");

    if (!m_floatMode)
    {
	int bps = WavpackGetBitsPerSample(m_context);

	if ((bps % 8) != 0)
	    bps = (bps + 7) & ~7;

	m_scale = (1 << (bps - 1)) - 1;
    }
}

// =====================================================================================================================
player::Format WavPack::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
bool WavPack::decode(float*& samples, size_t& count)
{
    bool ret = m_floatMode ? decodeFloat(count) : decodeInt(count);

    if (ret)
	samples = &m_samples[0];

    return ret;
}

// =====================================================================================================================
void WavPack::seek(off_t sample)
{
}

// =====================================================================================================================
std::unique_ptr<zeppelin::library::Metadata> WavPack::readMetadata()
{
    char error[100];

    m_context = WavpackOpenFileInput(m_file.c_str(), error, OPEN_WVC | OPEN_2CH_MAX /* whew ... */ | OPEN_NORMALIZE, 0);

    if (!m_context)
	throw CodecException("unable to open file");

    std::unique_ptr<zeppelin::library::Metadata> metadata(new zeppelin::library::Metadata("wv"));;

    metadata->setFormat(WavpackGetReducedChannels(m_context), // use GetNumChannels() once we support more than 2 ...
			WavpackGetSampleRate(m_context),
			WavpackGetBitsPerSample(m_context));
    metadata->setLength(WavpackGetNumSamples(m_context) / WavpackGetSampleRate(m_context));

    return metadata;
}

// =====================================================================================================================
bool WavPack::decodeInt(size_t& count)
{
    int32_t buffer[4096];

    count = WavpackUnpackSamples(m_context, buffer, sizeof(buffer) / sizeof(int32_t) / m_channels);

    if (count == 0)
	return false;

    m_samples.resize(count * m_channels);

    for (size_t i = 0; i < m_samples.size(); ++i)
    {
	float f = (float)buffer[i] / m_scale;

	if (f > 1.0f)
	    f = 1.0f;
	else if (f < -1.0f)
	    f = -1.0f;

	m_samples[i] = f;
    }

    return true;
}

// =====================================================================================================================
bool WavPack::decodeFloat(size_t& count)
{
    m_samples.resize(1024 * 2 /* 2 channels at most for now */);

    // in float mode we can directly decode into the float buffer
    count = WavpackUnpackSamples(
	m_context,
	reinterpret_cast<int32_t*>(&m_samples[0]),
	m_samples.size() / m_channels);

    return count > 0;
}
