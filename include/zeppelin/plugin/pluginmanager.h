#ifndef ZEPPELIN_PLUGIN_PLUGINMANAGER_H_INCLUDED
#define ZEPPELIN_PLUGIN_PLUGINMANAGER_H_INCLUDED

#include <string>
#include <stdexcept>

namespace zeppelin
{
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
	virtual ~PluginManager()
	{}

	virtual PluginInterface& getInterface(const std::string& name) = 0;
	virtual void registerInterface(const std::string& name, PluginInterface* pi) = 0;
};

}
}

#endif
