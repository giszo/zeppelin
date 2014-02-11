#include "pulseaudio.h"

#include <zeppelin/logger.h>

using output::PulseAudio;

// =====================================================================================================================
PulseAudio::PulseAudio(const config::Config& config)
    : BaseOutput(config, "pulseaudio"),
      m_rate(0),
      m_channels(0),
      m_mainloop(NULL),
      m_context(NULL),
      m_stream(NULL)
{
}

// =====================================================================================================================
player::Format PulseAudio::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
int PulseAudio::getFreeSize()
{
    int size;

    pa_threaded_mainloop_lock(m_mainloop);
    size = pa_stream_writable_size(m_stream) / m_channels / sizeof(float);
    pa_threaded_mainloop_unlock(m_mainloop);

    return size;
}

// =====================================================================================================================
void PulseAudio::setup(int rate, int channels)
{
    // create mainloop
    m_mainloop = pa_threaded_mainloop_new();

    if (!m_mainloop)
	throw OutputException("unable to create mainloop");

    // create context
    m_context = pa_context_new(pa_threaded_mainloop_get_api(m_mainloop), "zeppelin");

    if (!m_context)
	throw OutputException("unable to create context");

    // register our own state callback that simply notified the mainloop when something happens
    pa_context_set_state_callback(m_context, _contextStateCallback, this);

    if (pa_context_connect(m_context, NULL, PA_CONTEXT_NOFLAGS, NULL) != 0)
	throw OutputException("unable to connect context");

    pa_threaded_mainloop_lock(m_mainloop);

    if (pa_threaded_mainloop_start(m_mainloop) != 0)
        throw OutputException("unable to start threaded mainloop");

    // wait for the context to get ready
    pa_threaded_mainloop_wait(m_mainloop);

    if (pa_context_get_state(m_context) != PA_CONTEXT_READY)
        throw OutputException("context is not ready");

    pa_sample_spec ss;
    ss.format = PA_SAMPLE_FLOAT32LE;
    ss.rate = rate;
    ss.channels = channels;

    // create stream
    m_stream = pa_stream_new(m_context, "playback", &ss, NULL);

    if (!m_stream)
        throw OutputException("unable to create stream");

    // see context state callback above for details :]
    pa_stream_set_state_callback(m_stream, _streamStateCallback, this);

    // connect stream
    if (pa_stream_connect_playback(m_stream, NULL, NULL, PA_STREAM_NOFLAGS, NULL, NULL) != 0)
        throw OutputException("unable to connect stream");

    // wait until the stream is ready
    pa_threaded_mainloop_wait(m_mainloop);

    if (pa_stream_get_state(m_stream) != PA_STREAM_READY)
        throw OutputException("stream is not ready");

    pa_threaded_mainloop_unlock(m_mainloop);

    // save rate and channel informations
    m_rate = rate;
    m_channels = channels;
}

// =====================================================================================================================
void PulseAudio::prepare()
{
}

// =====================================================================================================================
void PulseAudio::drop()
{
    pa_operation_state_t state;

    pa_threaded_mainloop_lock(m_mainloop);

    pa_operation* op = pa_stream_flush(m_stream, _flushSuccessCallback, this);

    if (!op)
    {
	pa_threaded_mainloop_unlock(m_mainloop);
	LOG("pulseaudio: unable to flush stream!");
	return;
    }

    do
    {
	state = pa_operation_get_state(op);

	if (state == PA_OPERATION_RUNNING)
	    pa_threaded_mainloop_wait(m_mainloop);
    } while (state == PA_OPERATION_RUNNING);

    pa_operation_unref(op);

    pa_threaded_mainloop_unlock(m_mainloop);

    if (state == PA_OPERATION_CANCELLED)
	LOG("pulseaudio: stream flush operation was cancelled!");
}

// =====================================================================================================================
void PulseAudio::write(const float* samples, size_t count)
{
    int result;

    pa_threaded_mainloop_lock(m_mainloop);
    result = pa_stream_write(m_stream, samples, count * m_channels * sizeof(float), NULL, 0, PA_SEEK_RELATIVE);
    pa_threaded_mainloop_unlock(m_mainloop);

    if (result < 0)
	throw OutputException("unable to play samples");
}

// =====================================================================================================================
void PulseAudio::contextStateCallback(pa_context* context)
{
    switch (pa_context_get_state(context))
    {
	case PA_CONTEXT_READY :
	case PA_CONTEXT_TERMINATED :
	case PA_CONTEXT_FAILED :
	    pa_threaded_mainloop_signal(m_mainloop, 0);
	    break;

	default :
	    break;
    }
}

// =====================================================================================================================
void PulseAudio::streamStateCallback(pa_stream* stream)
{
    switch (pa_stream_get_state(stream))
    {
        case PA_STREAM_READY :
        case PA_STREAM_FAILED :
        case PA_STREAM_TERMINATED :
            pa_threaded_mainloop_signal(m_mainloop, 0);
            break;

	default :
	    break;
    }
}

// =====================================================================================================================
void PulseAudio::flushSuccessCallback(pa_stream* stream, int success)
{
    pa_threaded_mainloop_signal(m_mainloop, 0);
}

// =====================================================================================================================
void PulseAudio::_contextStateCallback(pa_context* context, void* p)
{
    PulseAudio* pa = reinterpret_cast<PulseAudio*>(p);
    pa->contextStateCallback(context);
}

// =====================================================================================================================
void PulseAudio::_streamStateCallback(pa_stream* stream, void* p)
{
    PulseAudio* pa = reinterpret_cast<PulseAudio*>(p);
    pa->streamStateCallback(stream);
}

// =====================================================================================================================
void PulseAudio::_flushSuccessCallback(pa_stream* stream, int success, void* p)
{
    PulseAudio* pa = reinterpret_cast<PulseAudio*>(p);
    pa->flushSuccessCallback(stream, success);
}
