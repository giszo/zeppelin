#include "vorbis.h"

#include <zeppelin/logger.h>

#include <endian.h>

using codec::Vorbis;

// =====================================================================================================================
Vorbis::Vorbis(const std::string& file)
    : BaseCodec(file),
      m_open(false),
      m_rate(0),
      m_channels(0)
{
}

// =====================================================================================================================
Vorbis::~Vorbis()
{
    if (m_open)
	ov_clear(&m_vf);
}

// =====================================================================================================================
void Vorbis::open()
{
    if (ov_fopen(m_file.c_str(), &m_vf) != 0)
	throw CodecException("unable to open file");

    m_open = true;

    vorbis_info* info = ov_info(&m_vf, -1);

    if (!info)
	throw CodecException("unable to get vorbis info");

    m_rate = info->rate;
    m_channels = info->channels;

    if (m_channels != 2)
    {
	LOG("vorbis: currently 2 channels are supported only!");
	throw CodecException("unsupported channels");
    }
}

// =====================================================================================================================
player::Format Vorbis::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
bool Vorbis::decode(float*& samples, size_t& count)
{
    int bitstream;
    char buf[BUFFER_SIZE];

    long ret = ov_read(
	&m_vf,
	buf,
	BUFFER_SIZE,
#if __BYTE_ORDER == __LITTLE_ENDIAN
	0,
#elif __BYTE_ORDER == __BIG_ENDIAN
	1,
#else
#error "invalid endian"
#endif
	2, /* 16 bit samples */
	1, /* signed samples */
	&bitstream);

    switch (ret)
    {
	case OV_HOLE :
	case OV_EBADLINK :
	    // TODO: treat OV_EBADLINK as an error?
	    count = 0;
	    return true;

	case OV_EINVAL :
	    // something invalid happened
	    return false;

	case 0 :
	    // end of file
	    return false;
    }

    // make sure we got valid amount of data
    if (ret % (2 /* bytes per sample */ * m_channels) != 0)
	throw CodecException("invalid amount of decoded data");

    // allocate buffer for float samples
    m_samples.resize(ret / 2);

    for (long i = 0; i < ret; i += 2)
    {
	int16_t* p = reinterpret_cast<int16_t*>(&buf[i]);
	float f = *p / 32767.0f;

	if (f > 1.0f)
	    f = 1.0f;
	else if (f < -1.0f)
	    f = -1.0f;

	m_samples[i / 2] = f;
    }

    samples = &m_samples[0];
    count = m_samples.size() / 2;

    return true;
}

// =====================================================================================================================
void Vorbis::seek(off_t sample)
{
    int ret = ov_pcm_seek(&m_vf, sample);

    switch (ret)
    {
	case OV_ENOSEEK :
	    LOG("vorbis: stream is not seekable");
	    break;
    }
}

// =====================================================================================================================
codec::Metadata Vorbis::readMetadata()
{
    Metadata m;
    m.m_codec = "ogg";

    if (ov_fopen(m_file.c_str(), &m_vf) != 0)
	throw CodecException("unable to open file");

    m_open = true;

    // sampling rate & channels
    vorbis_info* info = ov_info(&m_vf, -1);

    if (!info)
	throw CodecException("unable to get vorbis info");

    m.m_rate = info->rate;
    m.m_channels = info->channels;

    // total number of samples
    ogg_int64_t samples = ov_pcm_total(&m_vf, -1);

    if (samples == OV_EINVAL)
	throw CodecException("unable to get number of samples");

    m.m_samples = samples;

    // vorbis comment
    vorbis_comment* vc = ov_comment(&m_vf, -1);

    if (!vc)
	throw CodecException("unable to get vorbis comment");

    for (int i = 0; i < vc->comments; ++i)
	m.setVorbisComment(vc->user_comments[i]);

    return m;
}
