/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "eventlistenerproxy.h"

using player::EventListenerProxy;

// =====================================================================================================================
void EventListenerProxy::add(const std::shared_ptr<zeppelin::player::EventListener>& listener)
{
    m_listeners.insert(listener);
}

// =====================================================================================================================
void EventListenerProxy::started()
{
    for (const auto& l : m_listeners)
	l->started();
}

// =====================================================================================================================
void EventListenerProxy::paused()
{
    for (const auto& l : m_listeners)
	l->paused();
}

// =====================================================================================================================
void EventListenerProxy::stopped()
{
    for (const auto& l : m_listeners)
	l->stopped();
}

// =====================================================================================================================
void EventListenerProxy::positionChanged(unsigned pos)
{
    for (const auto& l : m_listeners)
	l->positionChanged(pos);
}

// =====================================================================================================================
void EventListenerProxy::songChanged(const std::vector<int>& idx)
{
    for (const auto& l : m_listeners)
	l->songChanged(idx);
}

// =====================================================================================================================
void EventListenerProxy::queueChanged()
{
    for (const auto& l : m_listeners)
	l->queueChanged();
}

// =====================================================================================================================
void EventListenerProxy::volumeChanged(int vol)
{
    for (const auto& l : m_listeners)
	l->volumeChanged(vol);
}
