#include "resample.h"

#include <cmath>

using filter::Resample;

// =====================================================================================================================
Resample::Resample(int srcRate, int dstRate)
    : m_srcRate(srcRate),
      m_dstRate(dstRate)
{
    int error;

    // TODO: resampling type should be configurable!
    m_src = src_new(SRC_SINC_BEST_QUALITY, 2, &error);
}

// =====================================================================================================================
Resample::~Resample()
{
    if (m_src)
	src_delete(m_src);
}

// =====================================================================================================================
void Resample::init()
{
    if (!m_src)
	throw FilterException("m_src is not present");

    // currently there is no way to detect the end of the input in a filter
    m_data.end_of_input = 0;
    m_data.src_ratio = (double)m_dstRate / m_srcRate;

    if (!src_is_valid_ratio(m_data.src_ratio))
	throw FilterException("invalid resampling ratio");
}

// =====================================================================================================================
void Resample::run(float*& samples, size_t& count, const player::Format& format)
{
    // prepare the output buffer to be able to store all of the samples
    m_samples.resize(count * ceil(m_data.src_ratio) * format.getChannels());

    m_data.data_in = samples;
    m_data.input_frames = count;
    m_data.data_out = &m_samples[0];
    m_data.output_frames = m_samples.size() / format.getChannels();

    if (src_process(m_src, &m_data) != 0)
	throw FilterException("resampling error");

    samples = &m_samples[0];
    count = m_data.output_frames_gen;
}
