#ifndef CODEC_MP3_H_INCLUDED
#define CODEC_MP3_H_INCLUDED

#include "basecodec.h"

#include <mpg123.h>

namespace codec
{

class Mp3 : public BaseCodec
{
    public:
	Mp3(const std::string& file);
	virtual ~Mp3();

	void open() override;

	int getRate() override;
	int getChannels() override;

	bool decode(int16_t*& samples, size_t& count) override;

	Metadata readMetadata() override;

    private:
	mpg123_handle* m_handle;

	long m_rate;
	int m_channels;
	int m_format;
};

}

#endif
