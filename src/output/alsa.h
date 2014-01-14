#ifndef OUTPUT_ALSA_H_INCLUDED
#define OUTPUT_ALSA_H_INCLUDED

#include "baseoutput.h"

#include <alsa/asoundlib.h>

namespace output
{

class AlsaOutput : public BaseOutput
{
    public:
	AlsaOutput(const config::Config& config);
	virtual ~AlsaOutput();

	player::Format getFormat() const override;

	int getFreeSize() override;

	void setup(int rate, int channels) override;

	void drop() override;

	void write(const float* samples, size_t count) override;

    private:
	void handleError(int error);

	std::string getPcmName() const;

    private:
	snd_pcm_t* m_handle;

	int m_rate;
	int m_channels;

	std::vector<int16_t> m_buffer;
};

}

#endif
