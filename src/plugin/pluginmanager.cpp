// TODO: proper cleanup of plugins should be implemented

#include "pluginmanager.h"

#include <zeppelin/logger.h>
#include <zeppelin/plugin/plugin.h>

#include <dlfcn.h>

using plugin::PluginManagerImpl;

// =====================================================================================================================
PluginManagerImpl::PluginManagerImpl(const std::shared_ptr<zeppelin::library::MusicLibrary>& library,
				     const std::shared_ptr<zeppelin::player::Controller>& controller)
    : m_library(library),
      m_controller(controller)
{
}

// =====================================================================================================================
void PluginManagerImpl::loadAll(const config::Config& config)
{
    const config::Plugins& plugins = config.m_plugins;

    for (const auto& name : plugins.m_enabled)
    {
	LOG("plugin: loading " << name);
	load(plugins.m_root + "/lib" + name + ".so", plugins.m_available.find(name)->second);
    }
}

// =====================================================================================================================
zeppelin::plugin::PluginInterface& PluginManagerImpl::getInterface(const std::string& name)
{
    auto it = m_interfaces.find(name);

    if (it == m_interfaces.end())
	throw zeppelin::plugin::PluginInterfaceNotFoundException(name);

    return *it->second;
}

// =====================================================================================================================
void PluginManagerImpl::registerInterface(const std::string& name, zeppelin::plugin::PluginInterface* pi)
{
    LOG("plugin: registering " << name);
    m_interfaces[name] = pi;
}

// =====================================================================================================================
void PluginManagerImpl::load(const std::string& path, const Json::Value& config)
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

    zeppelin::plugin::Plugin* plugin = create(m_library, m_controller);

    if (!plugin)
    {
	LOG("plugin: unable to create plugin for " << path);
	dlclose(p);
	return;
    }

    plugin->start(config, *this);
}
