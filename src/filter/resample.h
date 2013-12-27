#ifndef FILTER_RESAMPLE_H_INCLUDED
#define FILTER_RESAMPLE_H_INCLUDED

#include "basefilter.h"

#include <samplerate.h>

#include <vector>

namespace filter
{

class Resample : public BaseFilter
{
    public:
	Resample(int srcRate, int dstRate);
	virtual ~Resample();

	void init() override;
	void run(float*& samples, size_t& count, const player::Format& format) override;

    private:
	// source sampling rate
	int m_srcRate;
	// destination sampling rate
	int m_dstRate;

	SRC_STATE* m_src;
	SRC_DATA m_data;

	// buffer for resampled output
	std::vector<float> m_samples;
};

}

#endif
