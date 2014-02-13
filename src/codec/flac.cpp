#include "flac.h"

#include <library/vorbismetadata.h>

#include <zeppelin/logger.h>

#include <cstring>

using codec::Flac;

// =====================================================================================================================
Flac::Flac(const std::string& file)
    : BaseCodec(file),
      m_decoder(NULL),
      m_iterator(NULL),
      m_initialized(false),
      m_rate(0),
      m_channels(0),
      m_bps(0),
      m_scale(0),
      m_error(false)
{
}

// =====================================================================================================================
Flac::~Flac()
{
    if (m_decoder)
    {
	if (m_initialized)
	    FLAC__stream_decoder_finish(m_decoder);

	FLAC__stream_decoder_delete(m_decoder);
    }

    if (m_iterator)
	FLAC__metadata_simple_iterator_delete(m_iterator);
}

// =====================================================================================================================
void Flac::open()
{
    m_decoder = FLAC__stream_decoder_new();

    if (!m_decoder)
	throw CodecException("unable to create FLAC decoder");

    FLAC__stream_decoder_set_md5_checking(m_decoder, true);

    if (FLAC__stream_decoder_init_file(m_decoder,
				       m_file.c_str(),
				       _writeCallback,
				       _metadataCallback,
				       _errorCallback,
				       this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	throw CodecException("unable to open file");

    m_initialized = true;

    if (!FLAC__stream_decoder_process_until_end_of_metadata(m_decoder))
	throw CodecException("unable to process metadata");

    if (m_error)
    {
	LOG("flac: error was set after processing metadata");
	throw CodecException("unable to process metadata");
    }

    if (m_channels != 2)
    {
	LOG("flac: currently 2 channels are supported only!");
	throw CodecException("unsupported channels");
    }

    if (m_bps > 32)
    {
	LOG("flac: BPS not supported above 32 bits");
	throw CodecException("unsupported bps");
    }

    // calculate value used for scaling samples
    m_scale = (1 << (m_bps - 1)) - 1;
}

// =====================================================================================================================
player::Format Flac::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
std::unique_ptr<zeppelin::library::Metadata> Flac::readMetadata()
{
    std::unique_ptr<zeppelin::library::Metadata> metadata(new library::VorbisMetadata("flac"));

    m_iterator = FLAC__metadata_simple_iterator_new();

    if (!m_iterator)
	throw CodecException("unable to create metadata iterator");

    if (!FLAC__metadata_simple_iterator_init(m_iterator, m_file.c_str(), true, false))
	throw CodecException("unable to open file");

    do
    {
	switch (FLAC__metadata_simple_iterator_get_block_type(m_iterator))
	{
	    case FLAC__METADATA_TYPE_STREAMINFO :
	    {
		FLAC__StreamMetadata* meta = FLAC__metadata_simple_iterator_get_block(m_iterator);

		if (!meta)
		    break;

		metadata->setFormat(meta->data.stream_info.channels,
				    meta->data.stream_info.sample_rate,
				    meta->data.stream_info.bits_per_sample);
		metadata->setLength(meta->data.stream_info.total_samples / meta->data.stream_info.sample_rate);

		FLAC__metadata_object_delete(meta);

		break;
	    }

	    case FLAC__METADATA_TYPE_VORBIS_COMMENT :
	    {
		FLAC__StreamMetadata* meta = FLAC__metadata_simple_iterator_get_block(m_iterator);

		if (!meta)
		    break;

		for (FLAC__uint32 i = 0; i < meta->data.vorbis_comment.num_comments; ++i)
		{
		    FLAC__StreamMetadata_VorbisComment_Entry* entry = &meta->data.vorbis_comment.comments[i];
		    const char* data = reinterpret_cast<const char*>(entry->entry);

		    static_cast<library::VorbisMetadata&>(*metadata).setVorbisComment(data);
		}

		FLAC__metadata_object_delete(meta);

		break;
	    }

	    default :
		break;
	}
    } while (FLAC__metadata_simple_iterator_next(m_iterator));

    return metadata;
}

// =====================================================================================================================
bool Flac::decode(float*& samples, size_t& count)
{
    FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(m_decoder);

    switch (state)
    {
	case FLAC__STREAM_DECODER_END_OF_STREAM :
	case FLAC__STREAM_DECODER_OGG_ERROR :
	case FLAC__STREAM_DECODER_ABORTED :
	case FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR :
	    return false;

	default :
	    // decoding can be continued in the remaining states
	    break;
    }

    m_error = false;
    m_samples.clear();

    if (!FLAC__stream_decoder_process_single(m_decoder))
	throw CodecException("stream decoding error");

    samples = &m_samples[0];
    count = m_samples.size() / 2;

    return true;
}

// =====================================================================================================================
void Flac::seek(off_t sample)
{
    if (!FLAC__stream_decoder_seek_absolute(m_decoder, sample))
    {
	LOG("flac: seek error");
	// TODO: call FLAC__stream_decoder_flush() here?
    }
}

// =====================================================================================================================
static inline void convertSample(const FLAC__int32 in, float& out, unsigned scale)
{
    out = (float)in / scale;

    if (out > 1.0f)
	out = 1.0f;
    else if (out < -1.0f)
	out = -1.0f;
}

// =====================================================================================================================
FLAC__StreamDecoderWriteStatus Flac::writeCallback(const FLAC__Frame* frame,
						   const FLAC__int32* const buffer[])
{
    // reserve space to store the decoded samples
    m_samples.resize(frame->header.blocksize * 2);

    size_t idx = 0;

    // create an interleaved buffer of samples
    for (size_t i = 0; i < frame->header.blocksize; ++i)
    {
	convertSample(buffer[0][i], m_samples[idx++], m_scale);
	convertSample(buffer[1][i], m_samples[idx++], m_scale);
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

// =====================================================================================================================
void Flac::metadataCallback(const FLAC__StreamMetadata* metadata)
{
    switch (metadata->type)
    {
	case FLAC__METADATA_TYPE_STREAMINFO :
	    m_rate = metadata->data.stream_info.sample_rate;
	    m_channels = metadata->data.stream_info.channels;
	    m_bps = metadata->data.stream_info.bits_per_sample;
	    break;

	default :
	    // we are not interested in other type of metadata here
	    break;
    }
}

// =====================================================================================================================
void Flac::errorCallback(FLAC__StreamDecoderErrorStatus status)
{
    LOG("flac: error=" << status);
    m_error = true;
}

// =====================================================================================================================
FLAC__StreamDecoderWriteStatus Flac::_writeCallback(const FLAC__StreamDecoder* decoder,
						   const FLAC__Frame* frame,
						   const FLAC__int32* const buffer[],
						   void* clientData)
{
    return reinterpret_cast<Flac*>(clientData)->writeCallback(frame, buffer);
}

// =====================================================================================================================
void Flac::_metadataCallback(const FLAC__StreamDecoder* decoder,
			     const FLAC__StreamMetadata* metadata,
			     void* clientData)
{
    reinterpret_cast<Flac*>(clientData)->metadataCallback(metadata);
}

// =====================================================================================================================
void Flac::_errorCallback(const FLAC__StreamDecoder* decoder,
			  FLAC__StreamDecoderErrorStatus status,
			  void* clientData)
{
    reinterpret_cast<Flac*>(clientData)->errorCallback(status);
}
