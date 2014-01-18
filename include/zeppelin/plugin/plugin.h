#ifndef ZEPPELIN_PLUGIN_PLUGIN_H_INCLUDED
#define ZEPPELIN_PLUGIN_PLUGIN_H_INCLUDED

#include <jsoncpp/json/value.h>

namespace zeppelin
{
namespace plugin
{

class PluginManager;

class Plugin
{
    public:
	// returns the name of the plugin
	virtual std::string getName() const = 0;

	// called after loading the plugin to start it
	virtual void start(const Json::Value& config,
			   PluginManager& pm) = 0;

	// called before unloading the plugin to stop it
	virtual void stop() = 0;
};

}
}

#endif
