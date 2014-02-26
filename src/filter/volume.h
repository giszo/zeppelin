/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef FILTER_VOLUME_H_INCLUDED
#define FILTER_VOLUME_H_INCLUDED

#include "basefilter.h"

#include <atomic>

namespace filter
{

class Volume : public BaseFilter
{
    public:
	Volume(const config::Config& config);

	int getLevel() const;
	bool setLevel(int level);

	void init() override;
	void run(float*& samples, size_t& count, const player::Format& format) override;

    private:
	// level of the volume on a linear scale
	int m_linearLevel;

	// the value used for scaling samples
	std::atomic<float> m_level;
};

}

#endif
