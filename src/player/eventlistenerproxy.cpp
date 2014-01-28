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
void EventListenerProxy::positionChanged()
{
    for (const auto& l : m_listeners)
	l->positionChanged();
}

// =====================================================================================================================
void EventListenerProxy::songChanged()
{
    for (const auto& l : m_listeners)
	l->songChanged();
}

// =====================================================================================================================
void EventListenerProxy::queueChanged()
{
    for (const auto& l : m_listeners)
	l->queueChanged();
}

// =====================================================================================================================
void EventListenerProxy::volumeChanged()
{
    for (const auto& l : m_listeners)
	l->volumeChanged();
}
