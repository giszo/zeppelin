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

	void setLevel(float level);

	void init() override;
	void run(float*& samples, size_t& count, const player::Format& format) override;

    private:
	std::atomic<float> m_level;
};

}

#endif
