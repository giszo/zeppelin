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
