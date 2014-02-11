#ifndef OUTPUT_PULSEAUDIO_H_INCLUDED
#define OUTPUT_PULSEAUDIO_H_INCLUDED

#include "baseoutput.h"

#include <pulse/pulseaudio.h>

namespace output
{

class PulseAudio : public BaseOutput
{
    public:
	PulseAudio(const config::Config& config);

	player::Format getFormat() const override;

	int getFreeSize() override;

	void setup(int rate, int channels) override;

	void prepare() override;
	void drop() override;

	void write(const float* samples, size_t count) override;

    private:
	void contextStateCallback(pa_context* context);
	void streamStateCallback(pa_stream* stream);
	void flushSuccessCallback(pa_stream* stream, int success);

	static void _contextStateCallback(pa_context* context, void* p);
	static void _streamStateCallback(pa_stream* stream, void* p);
	static void _flushSuccessCallback(pa_stream* stream, int success, void* p);

    private:
	int m_rate;
	int m_channels;

	pa_threaded_mainloop* m_mainloop;
	pa_context* m_context;
	pa_stream* m_stream;
};

}

#endif
