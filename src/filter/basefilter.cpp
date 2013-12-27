#include "basefilter.h"

using filter::BaseFilter;

// =====================================================================================================================
BaseFilter::BaseFilter(const config::Config& config, const std::string& name)
{
    if (config.m_raw.isMember("filter") && config.m_raw["filter"].isMember(name))
	m_config = &config.m_raw["filter"][name];
    else
	m_config = NULL;

}

// =====================================================================================================================
bool BaseFilter::hasConfig() const
{
    return m_config != NULL;
}

// =====================================================================================================================
const Json::Value& BaseFilter::getConfig() const
{
    return *m_config;
}
