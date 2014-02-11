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
