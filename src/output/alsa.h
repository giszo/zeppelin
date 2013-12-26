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

	int getRate() override;
	int getChannels() override;

	int getFreeSize() override;

	void setup(int rate, int channels) override;

	void drop() override;

	void write(const int16_t* samples, size_t count) override;

    private:
	void handleError(int error);

	std::string getPcmName() const;

    private:
	snd_pcm_t* m_handle;

	int m_rate;
	int m_channels;
};

}

#endif
