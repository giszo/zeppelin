#ifndef PLUGIN_PLUGINMANAGER_H_INCLUDED
#define PLUGIN_PLUGINMANAGER_H_INCLUDED

#include <zeppelin/plugin/pluginmanager.h>
#include <zeppelin/library/musiclibrary.h>
#include <zeppelin/player/controller.h>

#include <config/config.h>

#include <unordered_map>

namespace zeppelin
{
namespace plugin
{
class Plugin;
}
}

namespace plugin
{

class PluginManagerImpl : public zeppelin::plugin::PluginManager
{
    public:
	PluginManagerImpl(const std::shared_ptr<zeppelin::library::MusicLibrary>& library,
			  const std::shared_ptr<zeppelin::player::Controller>& controller);

	void loadAll(const config::Config& config);

	zeppelin::plugin::PluginInterface& getInterface(const std::string& name) override;
	void registerInterface(const std::string& name, zeppelin::plugin::PluginInterface* pi) override;

    private:
	void load(const std::string& path, const Json::Value& config);

    private:
	typedef zeppelin::plugin::Plugin* PluginCreate(const std::shared_ptr<zeppelin::library::MusicLibrary>&,
						       const std::shared_ptr<zeppelin::player::Controller>&);

	std::shared_ptr<zeppelin::library::MusicLibrary> m_library;
	std::shared_ptr<zeppelin::player::Controller> m_controller;

	std::unordered_map<std::string, zeppelin::plugin::PluginInterface*> m_interfaces;
};

}

#endif
