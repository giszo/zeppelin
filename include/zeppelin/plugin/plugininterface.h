#ifndef ZEPPELIN_PLUGIN_PLUGININTERFACE_H_INCLUDED
#define ZEPPELIN_PLUGIN_PLUGININTERFACE_H_INCLUDED

namespace zeppelin
{
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
}

#endif
