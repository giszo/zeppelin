/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef ZEPPELIN_PLAYER_EVENTLISTENER_H_INCLUDED
#define ZEPPELIN_PLAYER_EVENTLISTENER_H_INCLUDED

#include <vector>

namespace zeppelin
{
namespace player
{

class EventListener
{
    public:
	virtual ~EventListener()
	{}

	// playback was started
	virtual void started() {}
	// playback was paused
	virtual void paused() {}
	// playback was stopped
	virtual void stopped() {}

	// Position in the current track changed (eg. seeking).
	// Note that this function is not called during normal playback!
	virtual void positionChanged(unsigned) {}
	// the current song changed
	virtual void songChanged(const std::vector<int>&) {}

	// contents of the queue changed (file was added or removed)
	virtual void queueChanged() {}

	// volume setting changed
	virtual void volumeChanged(int) {}
};

}
}

#endif
