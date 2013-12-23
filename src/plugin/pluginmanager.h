#ifndef PLUGIN_PLUGINMANAGER_H_INCLUDED
#define PLUGIN_PLUGINMANAGER_H_INCLUDED

#include "plugin.h"

#include <config/config.h>

namespace plugin
{

class PluginManager
{
    public:
	PluginManager(const std::shared_ptr<library::MusicLibrary>& library,
		      const std::shared_ptr<player::Controller>& controller);

	void loadAll(const config::Config& config);

    private:
	void load(const std::string& path, const config::Config& config);

    private:
	typedef Plugin* PluginCreate(const config::Config& config,
				     const std::shared_ptr<library::MusicLibrary>&,
				     const std::shared_ptr<player::Controller>&);

	std::shared_ptr<library::MusicLibrary> m_library;
	std::shared_ptr<player::Controller> m_controller;
};

}

#endif
