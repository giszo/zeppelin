#ifndef CODEC_MP3_H_INCLUDED
#define CODEC_MP3_H_INCLUDED

#include "basecodec.h"

#include <mpg123.h>

namespace codec
{

class Mp3 : public BaseCodec
{
    public:
	Mp3();
	virtual ~Mp3();

	void open(const std::string& file) override;

	int getRate() override;
	int getChannels() override;

	Metadata getMetadata() override;

	bool decode(int16_t*& samples, size_t& count) override;

    private:
	mpg123_handle* m_handle;

	long m_rate;
	int m_channels;
	int m_format;
};

}

#endif
