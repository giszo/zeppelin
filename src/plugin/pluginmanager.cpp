// TODO: proper cleanup of plugins should be implemented

#include "pluginmanager.h"

#include <iostream>

#include <dlfcn.h>

using plugin::PluginManager;

// =====================================================================================================================
PluginManager::PluginManager(const std::shared_ptr<library::MusicLibrary>& library,
			     const std::shared_ptr<player::Controller>& controller)
    : m_library(library),
      m_controller(controller)
{
}

// =====================================================================================================================
void PluginManager::loadAll(const config::Config& config)
{
    for (const auto& name : config.m_plugin.m_list)
    {
	std::cout << "plugin: loading " << name << std::endl;
	load(config.m_plugin.m_root + "/lib" + name + ".so", config);
    }
}

// =====================================================================================================================
void PluginManager::load(const std::string& path, const config::Config& config)
{
    void* p;

    // open the plugin file
    p = dlopen(path.c_str(), RTLD_NOW);

    if (!p)
    {
	std::cout << "plugin: unable to open " << path << std::endl;
	std::cout << "error: " << dlerror() << std::endl;
	return;
    }

    // lookup its entry function
    PluginCreate* create = reinterpret_cast<PluginCreate*>(dlsym(p, "plugin_create"));

    if (!create)
    {
	std::cout << "plugin: create method not found in " << path << std::endl;
	dlclose(p);
	return;
    }

    Plugin* plugin = create(config, m_library, m_controller);

    if (!plugin)
    {
	std::cout << "plugin: unable to create plugin for " << path << std::endl;
	dlclose(p);
	return;
    }

    plugin->start();
}
