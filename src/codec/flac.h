#ifndef CODEC_FLAC_H_INCLUDED
#define CODEC_FLAC_H_INCLUDED

#include "basecodec.h"

#include <FLAC/stream_decoder.h>
#include <FLAC/metadata.h>

#include <vector>

namespace codec
{

class Flac : public BaseCodec
{
    public:
	Flac(const std::string& file);
	virtual ~Flac();

	void open() override;

	int getRate() override;
	int getChannels() override;

	bool decode(int16_t*& samples, size_t& count) override;

	Metadata readMetadata() override;

    private:
	FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__Frame* frame,
						     const FLAC__int32* const buffer[]);
	void metadataCallback(const FLAC__StreamMetadata* metadata);
	void errorCallback(FLAC__StreamDecoderErrorStatus status);

	void parseVorbisComment(const FLAC__StreamMetadata_VorbisComment& vc);

	static FLAC__StreamDecoderWriteStatus _writeCallback(const FLAC__StreamDecoder* decoder,
							    const FLAC__Frame* frame,
							    const FLAC__int32* const buffer[],
							    void* clientData);
	static void _metadataCallback(const FLAC__StreamDecoder* decoder,
				      const FLAC__StreamMetadata* metadata,
				      void* clientDdata);
	static void _errorCallback(const FLAC__StreamDecoder* decoder,
				   FLAC__StreamDecoderErrorStatus status,
				   void* clientData);

    private:
	FLAC__StreamDecoder* m_decoder;

	FLAC__Metadata_SimpleIterator* m_iterator;

	bool m_initialized;

	int m_rate;
	int m_channels;
	int m_bps;

	bool m_error;
	std::vector<int16_t> m_samples;

	Metadata m_metadata;
};

}

#endif
