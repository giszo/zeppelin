#include "mp3.h"

#include <utils/stringutils.h>
#include <logger.h>

#include <cstring>

using codec::Mp3;
using codec::CodecException;
using utils::StringUtils;

// =====================================================================================================================
Mp3::Mp3(const std::string& file)
    : BaseCodec(file),
      m_handle(NULL),
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
void Mp3::open()
{
    // create handle
    create();

    // issue the first mpg123_decode_frame() call, it will detect the file format only, no samples will be decoded

    off_t frame;
    unsigned char* data;
    size_t size;

    if (mpg123_decode_frame(m_handle, &frame, &data, &size) != MPG123_NEW_FORMAT)
	throw CodecException("unable to detect file format");

    if (mpg123_getformat(m_handle, &m_rate, &m_channels, &m_format) != 0)
	throw CodecException("unable to get file format");

    if (m_channels != 2)
    {
	LOG("mp3: currently 2 channels are supported only!");
	throw CodecException("unsupported channels");
    }

    if (m_format != MPG123_ENC_SIGNED_16)
    {
	LOG("mp3: currently 16bit samples are supported only!");
	throw CodecException("unsupported BPS");
    }
}

// =====================================================================================================================
player::Format Mp3::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
codec::Metadata Mp3::readMetadata()
{
    Metadata info;

    // create handle
    create();

    if (mpg123_scan(m_handle) != MPG123_OK)
	throw CodecException("unable to scan media");

    if (mpg123_getformat(m_handle, &m_rate, &info.m_channels, NULL) != MPG123_OK)
	throw CodecException("unable to get file format");

    info.m_rate = m_rate;

    off_t samples = mpg123_length(m_handle);

    if (samples == MPG123_ERR)
	throw CodecException("unable to get media length");

    info.m_samples = samples;

    mpg123_id3v1* id3_v1;
    mpg123_id3v2* id3_v2;

    if (mpg123_id3(m_handle, &id3_v1, &id3_v2) != MPG123_OK)
	throw CodecException("unable to get id3");

    if (id3_v2)
	processID3v2(info, *id3_v2);
    if (id3_v1)
	processID3v1(info, *id3_v1);

    info.m_type = MP3;

    return info;
}

// =====================================================================================================================
bool Mp3::decode(float*& samples, size_t& count)
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

    int16_t* p = reinterpret_cast<int16_t*>(data);
    count = bytes / (m_channels * sizeof(int16_t));

    // convert samples
    m_samples.resize(count * m_channels);

    for (size_t i = 0; i < m_samples.size(); ++i)
    {
	m_samples[i] = (float)p[i] / 32767.0f;

	if (m_samples[i] > 1.0f)
	    m_samples[i] = 1.0f;
	else if (m_samples[i] < -1.0f)
	    m_samples[i] = -1.0f;
    }

    samples = &m_samples[0];

    return true;
}

// =====================================================================================================================
void Mp3::create()
{
    m_handle = mpg123_new(NULL, NULL);

    if (!m_handle)
	throw CodecException("unable to create handle");

    // turn the fancy error messages of mpg123 off
    mpg123_param(m_handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0);

    if (mpg123_open(m_handle, m_file.c_str()) != 0)
	throw CodecException("unable to open file");
}

// =====================================================================================================================
static inline void readID3v1Field(const char* field, size_t length, std::string& s)
{
    char buf[length + 1];

    // copy the data into a temporary buffer to add a zero byte at the end of the text
    memcpy(buf, field, length);
    buf[length] = 0;

    s = buf;
}

// =====================================================================================================================
void Mp3::processID3v1(Metadata& info, const mpg123_id3v1& id3)
{
    if (info.m_artist.empty())
	readID3v1Field(id3.artist, 30, info.m_artist);

    if (info.m_album.empty())
	readID3v1Field(id3.album, 30, info.m_album);

    if (info.m_title.empty())
	readID3v1Field(id3.title, 30, info.m_title);

    if (info.m_year == 0)
    {
	try
	{
	    info.m_year = StringUtils::toInt(std::string(id3.year, 4));
	}
	catch (const utils::NumberFormatException&)
	{
	}
    }

    // track index could be filled from ID3v1.1 data, but I had a lot of trouble with it, so it is skipped for now
}

// =====================================================================================================================
void Mp3::processID3v2(Metadata& info, const mpg123_id3v2& id3)
{
    if (id3.artist && id3.artist->p)
	info.m_artist = id3.artist->p;

    if (id3.album && id3.album->p)
	info.m_album = id3.album->p;

    if (id3.title && id3.title->p)
	info.m_title = id3.title->p;

    if (id3.year && id3.year->p)
    {
	try
	{
	    info.m_year = StringUtils::toInt(id3.year->p);
	}
	catch (const utils::NumberFormatException&)
	{
	}
    }

    for (size_t i = 0; i < id3.texts; ++i)
    {
	const mpg123_text& t = id3.text[i];

	if ((memcmp(t.id, "TRCK", 4) == 0) && t.text.p)
	{
	    try
	    {
		info.m_trackIndex = StringUtils::toInt(t.text.p);
	    }
	    catch (const utils::NumberFormatException&)
	    {
	    }
	}
    }
}
