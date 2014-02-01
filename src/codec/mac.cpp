#include "mac.h"

#include <zeppelin/logger.h>

#define DLLEXPORT
#include <mac/NoWindows.h>
#include <mac/MACLib.h>
#include <mac/CharacterHelper.h>

using codec::Mac;

// =====================================================================================================================
Mac::Mac(const std::string& file)
    : BaseCodec(file),
      m_decompress(NULL),
      m_rate(0),
      m_channels(0),
      m_bps(0)
{
}

// =====================================================================================================================
Mac::~Mac()
{
    if (m_decompress)
	delete m_decompress;
}

// =====================================================================================================================
void Mac::open()
{
    int error;

    str_utf16* file = GetUTF16FromANSI(m_file.c_str());
    m_decompress = CreateIAPEDecompress(file, &error);
    delete[] file;

    if (!m_decompress)
	throw CodecException("unable to open file");

    m_rate = m_decompress->GetInfo(APE_INFO_SAMPLE_RATE);
    m_channels = m_decompress->GetInfo(APE_INFO_CHANNELS);

    if (m_channels != 2)
    {
	LOG("mac: unsupported channels: " << m_channels);
	throw CodecException("unsupported channels");
    }

    m_bps = m_decompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
}

// =====================================================================================================================
player::Format Mac::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
static inline void convertSample(int32_t in, float& out, unsigned scale)
{
    out = (float)in / scale;

    if (out > 1.0f)
	out = 1.0f;
    else if (out < -1.0f)
	out = -1.0f;
}

// =====================================================================================================================
bool Mac::decode(float*& samples, size_t& count)
{
    const int blocks = 1024;

    int retrieved;
    int32_t buf[blocks * m_channels];

    m_decompress->GetData(reinterpret_cast<char*>(buf), blocks, &retrieved);

    if (!retrieved)
	return false;

    m_samples.resize(retrieved * m_channels);

    switch (m_bps)
    {
	case 8 :
	{
	    int8_t* p = reinterpret_cast<int8_t*>(buf);

	    for (int i = 0; i < retrieved * m_channels; ++i)
		convertSample(*p++, m_samples[i], 0x7f);

	    break;
	}

	case 16 :
	{
	    int16_t* p = reinterpret_cast<int16_t*>(buf);

	    for (int i = 0; i < retrieved * m_channels; ++i)
		convertSample(*p++, m_samples[i], 0x7fff);

	    break;
	}

	case 32 :
	{
	    for (int i = 0; i < retrieved * m_channels; ++i)
		convertSample(buf[i], m_samples[i], 0x7fffffff);

	    break;
	}
    }

    samples = &m_samples[0];
    count = retrieved;

    return true;
}

// =====================================================================================================================
void Mac::seek(off_t sample)
{
    m_decompress->Seek(sample);
}

// =====================================================================================================================
codec::Metadata Mac::readMetadata()
{
    codec::Metadata m;

    int error;
    str_utf16* file = GetUTF16FromANSI(m_file.c_str());
    m_decompress = CreateIAPEDecompress(file, &error);
    delete[] file;

    if (!m_decompress)
	throw CodecException("unable to open file");

    m.m_codec = "APE";
    m.m_rate = m_decompress->GetInfo(APE_INFO_SAMPLE_RATE);
    m.m_channels = m_decompress->GetInfo(APE_INFO_CHANNELS);
    m.m_sampleSize = m_decompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
    m.m_samples = (size_t)m_decompress->GetInfo(APE_DECOMPRESS_LENGTH_MS) * m.m_rate / 1000;

    return m;
}
