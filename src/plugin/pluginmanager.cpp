// TODO: proper cleanup of plugins should be implemented

#include "pluginmanager.h"

#include <logger.h>

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
    const config::Plugins& plugins = config.m_plugins;

    for (const auto& name : plugins.m_enabled)
    {
	LOG("plugin: loading " << name);
	load(plugins.m_root + "/lib" + name + ".so", plugins.m_available.find(name)->second);
    }
}

// =====================================================================================================================
auto PluginManager::getInterface(const std::string& name) -> PluginInterface&
{
    auto it = m_interfaces.find(name);

    if (it == m_interfaces.end())
	throw PluginInterfaceNotFoundException(name);

    return *it->second;
}

// =====================================================================================================================
void PluginManager::registerInterface(const std::string& name, PluginInterface* pi)
{
    LOG("plugin: registering " << name);
    m_interfaces[name] = pi;
}

// =====================================================================================================================
void PluginManager::load(const std::string& path, const Json::Value& config)
{
    void* p;

    // open the plugin file
    p = dlopen(path.c_str(), RTLD_NOW);

    if (!p)
    {
	LOG("plugin: unable to open " << path << "\n" << "error: " << dlerror());
	return;
    }

    // lookup its entry function
    PluginCreate* create = reinterpret_cast<PluginCreate*>(dlsym(p, "plugin_create"));

    if (!create)
    {
	LOG("plugin: create method not found in " << path);
	dlclose(p);
	return;
    }

    Plugin* plugin = create(m_library, m_controller);

    if (!plugin)
    {
	LOG("plugin: unable to create plugin for " << path);
	dlclose(p);
	return;
    }

    plugin->start(config, *this);
}
