#ifndef OUTPUT_ALSA_H_INCLUDED
#define OUTPUT_ALSA_H_INCLUDED

#include "baseoutput.h"

#include <alsa/asoundlib.h>

namespace output
{

class AlsaOutput : public BaseOutput
{
    public:
	AlsaOutput();
	virtual ~AlsaOutput();

	int getRate() override;
	int getChannels() override;

	int getAvailableSize() override;

	void setup(int rate, int channels) override;

	void start() override;
	void stop() override;

	void write(const int16_t* samples, size_t count) override;

    private:
	snd_pcm_t* m_handle;

	int m_rate;
	int m_channels;
};

}

#endif
