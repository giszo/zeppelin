#ifndef PLUGIN_PLUGININTERFACE_H_INCLUDED
#define PLUGIN_PLUGININTERFACE_H_INCLUDED

namespace plugin
{

class PluginInterface
{
    public:
	virtual ~PluginInterface()
	{}

	virtual int version() const = 0;
};

}

#endif
