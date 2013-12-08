#include "alsa.h"

using output::AlsaOutput;
using output::OutputException;

// =====================================================================================================================
AlsaOutput::AlsaOutput()
    : m_handle(NULL),
      m_rate(0),
      m_channels(0)
{
}

// =====================================================================================================================
AlsaOutput::~AlsaOutput()
{
    snd_pcm_close(m_handle);
}

// =====================================================================================================================
int AlsaOutput::getRate()
{
    return m_rate;
}

// =====================================================================================================================
int AlsaOutput::getChannels()
{
    return m_channels;
}

// =====================================================================================================================
int AlsaOutput::getAvailableSize()
{
    return snd_pcm_avail_update(m_handle);
}

// =====================================================================================================================
void AlsaOutput::setup(int rate, int channels)
{
    if (snd_pcm_open(&m_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) != 0)
	throw OutputException("unable to open PCM device");

    snd_pcm_hw_params_t* params;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(m_handle, params);
    snd_pcm_hw_params_set_access(m_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(m_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(m_handle, params, channels);
    snd_pcm_hw_params_set_rate(m_handle, params, rate, 0);

    if (snd_pcm_hw_params(m_handle, params) != 0)
	throw OutputException("unable to set HW parameters");

    m_rate = rate;
    m_channels = channels;
}

// =====================================================================================================================
void AlsaOutput::start()
{
    snd_pcm_prepare(m_handle);
}

// =====================================================================================================================
void AlsaOutput::stop()
{
    snd_pcm_drop(m_handle);
}

// =====================================================================================================================
void AlsaOutput::write(const int16_t* samples, size_t count)
{
    snd_pcm_writei(m_handle, samples, count);
}
