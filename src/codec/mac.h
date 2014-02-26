/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CODEC_MAC_H_INCLUDED
#define CODEC_MAC_H_INCLUDED

#include "basecodec.h"

#include <vector>

class IAPEDecompress;

namespace codec
{

class Mac : public BaseCodec
{
    public:
	Mac(const std::string& file);
	~Mac();

	void open() override;

	player::Format getFormat() const override;

	bool decode(float*& samples, size_t& count) override;

	void seek(off_t sample) override;

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override;

    private:
	IAPEDecompress* m_decompress;

	int m_rate;
	int m_channels;
	int m_bps;

	std::vector<float> m_samples;
};

}

#endif
