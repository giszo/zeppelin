#ifndef ZEPPELIN_PLAYER_EVENTLISTENER_H_INCLUDED
#define ZEPPELIN_PLAYER_EVENTLISTENER_H_INCLUDED

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
	virtual void positionChanged() {}
	// the current song changed
	virtual void songChanged() {}

	// contents of the queue changed (file was added or removed)
	virtual void queueChanged() {}

	// volume setting changed
	virtual void volumeChanged() {}
};

}
}

#endif
