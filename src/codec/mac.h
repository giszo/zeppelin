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

	Metadata readMetadata() override;

    private:
	IAPEDecompress* m_decompress;

	int m_rate;
	int m_channels;
	int m_bps;

	std::vector<float> m_samples;
};

}

#endif