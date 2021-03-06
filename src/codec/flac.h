/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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

	player::Format getFormat() const override;

	bool decode(float*& samples, size_t& count) override;

	void seek(off_t sample) override;

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override;

    private:
	FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__Frame* frame,
						     const FLAC__int32* const buffer[]);
	void metadataCallback(const FLAC__StreamMetadata* metadata);
	void errorCallback(FLAC__StreamDecoderErrorStatus status);

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

	// value used for scaling samples to the -1.0 ... 1.0 range
	unsigned m_scale;

	bool m_error;
	std::vector<float> m_samples;
};

}

#endif
