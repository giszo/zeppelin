/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

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
