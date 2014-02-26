/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "baseoutput.h"

using output::BaseOutput;

// =====================================================================================================================
BaseOutput::BaseOutput(const config::Config& config, const std::string& name)
{
    if (config.m_raw.isMember("output") && config.m_raw["output"].isMember(name))
	m_config = &config.m_raw["output"][name];
    else
	m_config = NULL;
}

// =====================================================================================================================
bool BaseOutput::hasConfig() const
{
    return m_config != NULL;
}

// =====================================================================================================================
const Json::Value& BaseOutput::getConfig() const
{
    return *m_config;
}
