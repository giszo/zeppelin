#include "alsa.h"

#include <thread/thread.h>
#include <utils/makestring.h>

using output::AlsaOutput;
using output::OutputException;

// =====================================================================================================================
AlsaOutput::AlsaOutput(const config::Config& config)
    : BaseOutput(config, "alsa"),
      m_handle(NULL),
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
player::Format AlsaOutput::getFormat() const
{
    return player::Format(m_rate, m_channels);
}

// =====================================================================================================================
int AlsaOutput::getFreeSize()
{
    snd_pcm_sframes_t frames;

    while (1)
    {
	frames = snd_pcm_avail(m_handle);

	if (frames < 0)
	    handleError(frames);
	else
	    return frames;
    }
}

// =====================================================================================================================
void AlsaOutput::setup(int rate, int channels)
{
    if (snd_pcm_open(&m_handle, getPcmName().c_str(), SND_PCM_STREAM_PLAYBACK, 0) != 0)
	throw OutputException("unable to open PCM device");

    snd_pcm_hw_params_t* params;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(m_handle, params);
    snd_pcm_hw_params_set_access(m_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(m_handle, params, SND_PCM_FORMAT_S16);
    snd_pcm_hw_params_set_channels(m_handle, params, channels);
    snd_pcm_hw_params_set_rate(m_handle, params, rate, 0);

    if (hasConfig())
    {
	const Json::Value& cfg = getConfig();

	if (cfg.isMember("buffer-max"))
	{
	    snd_pcm_uframes_t frames = cfg["buffer-max"].asInt();
	    snd_pcm_hw_params_set_buffer_size_max(m_handle, params, &frames);
	}
    }

    if (snd_pcm_hw_params(m_handle, params) != 0)
	throw OutputException("unable to set HW parameters");

    m_rate = rate;
    m_channels = channels;
}

// =====================================================================================================================
void AlsaOutput::drop()
{
    snd_pcm_drop(m_handle);
    snd_pcm_prepare(m_handle);
}

// =====================================================================================================================
void AlsaOutput::write(const float* samples, size_t count)
{
    int16_t ss[count * m_channels];

    // convert float samples to signed 16bit integer
    for (size_t i = 0; i < count * m_channels; ++i)
	ss[i] = samples[i] * 32767;

    const int16_t* data = ss;

    while (count > 0)
    {
	snd_pcm_sframes_t ret = snd_pcm_writei(m_handle, data, count);

	if (ret < 0)
	    handleError(ret);
	else
	{
	    data += ret * m_channels;
	    count -= ret;
	}
    }
}

// =====================================================================================================================
void AlsaOutput::handleError(int error)
{
    if (error == -EAGAIN)
	return;
    else if (error == -EPIPE)
    {
	if (snd_pcm_prepare(m_handle) < 0)
	    throw OutputException("can't recover from underrun");
    }
    else if (error == -ESTRPIPE)
    {
	// wait until suspend flag is released
	while (snd_pcm_resume(m_handle) == -EAGAIN)
	    thread::Thread::sleep(100 * 1000);
	if (snd_pcm_prepare(m_handle) < 0)
	    throw OutputException("can't recover from suspend");
    }
    else
	throw OutputException(utils::MakeString() << "unable to write samples: " << error);
}

// =====================================================================================================================
std::string AlsaOutput::getPcmName() const
{
    if (!hasConfig())
	return "default";

    const Json::Value& cfg = getConfig();

    if (!cfg.isMember("pcm"))
	return "default";

    return cfg["pcm"].asString();
}
