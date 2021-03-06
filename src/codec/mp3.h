/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef CODEC_MP3_H_INCLUDED
#define CODEC_MP3_H_INCLUDED

#include "basecodec.h"

#include <mpg123.h>

#include <vector>

namespace codec
{

class Mp3 : public BaseCodec
{
    public:
	Mp3(const std::string& file);
	virtual ~Mp3();

	void open() override;

	player::Format getFormat() const override;

	bool decode(float*& samples, size_t& count) override;

	void seek(off_t sample) override;

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override;

    private:
	// creates the mpg123 handle for the given file
	void create(bool picture = false);

	void processID3v1(zeppelin::library::Metadata& info, const mpg123_id3v1& id3);
	void processID3v2(zeppelin::library::Metadata& info, const mpg123_id3v2& id3);

    private:
	mpg123_handle* m_handle;

	long m_rate;
	int m_channels;
	int m_format;

	std::vector<float> m_samples;
};

}

#endif
