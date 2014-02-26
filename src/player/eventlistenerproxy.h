/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef PLAYER_EVENTLISTENERPROXY_H_INCLUDED
#define PLAYER_EVENTLISTENERPROXY_H_INCLUDED

#include <zeppelin/player/eventlistener.h>

#include <set>
#include <memory>

namespace player
{

class EventListenerProxy : public zeppelin::player::EventListener
{
    public:
	void add(const std::shared_ptr<zeppelin::player::EventListener>& listener);

	void started() override;
	void paused() override;
	void stopped() override;

	void positionChanged(unsigned pos) override;
	void songChanged(const std::vector<int>& idx) override;

	void queueChanged() override;

	void volumeChanged(int vol) override;

    private:
	std::set<std::shared_ptr<zeppelin::player::EventListener>> m_listeners;
};

}

#endif
