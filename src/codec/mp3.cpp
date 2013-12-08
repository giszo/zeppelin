#include "mp3.h"

using codec::Mp3;
using codec::CodecException;

// =====================================================================================================================
Mp3::Mp3()
    : m_handle(NULL),
      m_rate(0),
      m_channels(0),
      m_format(0)
{
}

// =====================================================================================================================
Mp3::~Mp3()
{
    if (m_handle)
	mpg123_delete(m_handle);
}

// =====================================================================================================================
void Mp3::open(const std::string& file)
{
    m_handle = mpg123_new(NULL, NULL);

    if (!m_handle)
	throw CodecException("unable to create handle");

    if (mpg123_open(m_handle, file.c_str()) != 0)
	throw CodecException("unable to open file");

    // issue the first mpg123_decode_frame() call, it will detect the file format only, no samples will be decoded

    off_t frame;
    unsigned char* data;
    size_t size;

    if (mpg123_decode_frame(m_handle, &frame, &data, &size) != MPG123_NEW_FORMAT)
	throw CodecException("unable to detect file format");

    mpg123_getformat(m_handle, &m_rate, &m_channels, &m_format);
}

// =====================================================================================================================
int Mp3::getRate()
{
    return m_rate;
}

// =====================================================================================================================
int Mp3::getChannels()
{
    return m_channels;
}

// =====================================================================================================================
codec::MediaInfo Mp3::getMediaInfo()
{
    MediaInfo info;

    info.m_rate = m_rate;
    info.m_channels = m_channels;

    if (mpg123_scan(m_handle) != MPG123_OK)
	throw CodecException("unable to scan media");

    off_t samples = mpg123_length(m_handle);

    if (samples == MPG123_ERR)
	throw CodecException("unable to get media length");

    info.m_samples = samples;

    return info;
}

// =====================================================================================================================
bool Mp3::decode(int16_t*& samples, size_t& count)
{
    off_t frame;
    unsigned char* data;
    size_t bytes;

    // decode the next frame
    int r = mpg123_decode_frame(m_handle, &frame, &data, &bytes);

    if (r != MPG123_OK)
    {
	// check whether the end of the stream has been reached
	if (r == MPG123_DONE)
	    return false;

	throw CodecException("frame decoding error");
    }

    if ((bytes % (m_channels * sizeof(int16_t))) != 0)
	throw CodecException("invalid number of decoded bytes");

    samples = reinterpret_cast<int16_t*>(data);
    count = bytes / (m_channels * sizeof(int16_t));

    return true;
}
