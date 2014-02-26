/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CODEC_WAVPACK_H_INCLUDED
#define CODEC_WAVPACK_H_INCLUDED

#include "basecodec.h"

#include <wavpack/wavpack.h>

#include <vector>

namespace codec
{

class WavPack : public BaseCodec
{
    public:
	WavPack(const std::string& file);
	~WavPack();

	void open() override;

	player::Format getFormat() const override;

	bool decode(float*& samples, size_t& count) override;

	void seek(off_t sample) override;

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override;

    private:
	bool decodeInt(size_t& count);
	bool decodeFloat(size_t& count);

    private:
	WavpackContext* m_context;

	bool m_floatMode;

	int m_rate;
	int m_channels;

	unsigned m_scale;

	std::vector<float> m_samples;
};

}

#endif
