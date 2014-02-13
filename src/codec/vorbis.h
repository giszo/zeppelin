#ifndef CODEC_VORBIS_H_INCLUDED
#define CODEC_VORBIS_H_INCLUDED

#include "basecodec.h"

#include <vorbis/vorbisfile.h>

#include <vector>

namespace codec
{

class Vorbis : public BaseCodec
{
    public:
	Vorbis(const std::string& file);
	~Vorbis();

	void open() override;

	player::Format getFormat() const override;

	bool decode(float*& samples, size_t& count) override;

	void seek(off_t sample) override;

	std::unique_ptr<zeppelin::library::Metadata> readMetadata() override;

    private:
	// true when a vorbis file is currently opened
	bool m_open;
	OggVorbis_File m_vf;

	int m_rate;
	int m_channels;

	std::vector<float> m_samples;

	static const size_t BUFFER_SIZE = 4096;
};

}

#endif
