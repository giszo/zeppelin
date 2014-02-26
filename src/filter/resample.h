/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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
	Resample(int srcRate, int dstRate, const config::Config& config);
	virtual ~Resample();

	void init() override;
	void run(float*& samples, size_t& count, const player::Format& format) override;

    private:
	int getQuality() const;

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
