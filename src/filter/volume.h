#ifndef FILTER_VOLUME_H_INCLUDED
#define FILTER_VOLUME_H_INCLUDED

#include "basefilter.h"

#include <atomic>

namespace filter
{

class Volume : public BaseFilter
{
    public:
	Volume();

	void setLevel(float level);

	void run(float* samples, size_t count) override;

    private:
	std::atomic<float> m_level;
};

}

#endif
