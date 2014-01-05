#ifndef PLUGIN_PLUGINMANAGER_H_INCLUDED
#define PLUGIN_PLUGINMANAGER_H_INCLUDED

#include "plugin.h"

#include <config/config.h>

#include <unordered_map>
#include <stdexcept>

namespace plugin
{

class PluginInterface;

class PluginInterfaceNotFoundException : public std::runtime_error
{
    public:
	PluginInterfaceNotFoundException(const std::string& error)
	    : runtime_error(error)
	{}
};

class PluginManager
{
    public:
	PluginManager(const std::shared_ptr<library::MusicLibrary>& library,
		      const std::shared_ptr<player::Controller>& controller);

	void loadAll(const config::Config& config);

	PluginInterface& getInterface(const std::string& name);

	void registerInterface(const std::string& name, PluginInterface* pi);

    private:
	void load(const std::string& path, const Json::Value& config);

    private:
	typedef Plugin* PluginCreate(const std::shared_ptr<library::MusicLibrary>&,
				     const std::shared_ptr<player::Controller>&);

	std::shared_ptr<library::MusicLibrary> m_library;
	std::shared_ptr<player::Controller> m_controller;

	std::unordered_map<std::string, PluginInterface*> m_interfaces;
};

}

#endif
