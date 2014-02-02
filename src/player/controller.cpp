#include "controller.h"

#include <codec/codecmanager.h>
#include <output/alsa.h>
#include <thread/blocklock.h>

#include <zeppelin/logger.h>

#include <sstream>

using player::ControllerImpl;

// =====================================================================================================================
std::shared_ptr<ControllerImpl> ControllerImpl::create(const codec::CodecManager& codecManager,
						       const std::shared_ptr<Decoder>& decoder,
						       const std::shared_ptr<Player>& player,
						       const config::Config& config)
{
    std::shared_ptr<ControllerImpl> ctrl(new ControllerImpl(codecManager, decoder, player, config));
    ctrl->m_selfRef = ctrl;
    ctrl->init();
    return ctrl;
}

// =====================================================================================================================
ControllerImpl::ControllerImpl(const codec::CodecManager& codecManager,
			       const std::shared_ptr<Decoder>& decoder,
			       const std::shared_ptr<Player>& player,
			       const config::Config& config)
    : m_state(STOPPED),
      m_decoderInitialized(false),
      m_decoder(decoder),
      m_player(player),
      m_codecManager(codecManager)
{
}

// =====================================================================================================================
void ControllerImpl::init()
{
    // set our reference to the decoder and player
    m_decoder->setController(m_selfRef);
    m_player->setController(m_selfRef);
}

// =====================================================================================================================
void ControllerImpl::addListener(const std::shared_ptr<zeppelin::player::EventListener>& listener)
{
    m_listenerProxy.add(listener);
}

// =====================================================================================================================
std::shared_ptr<zeppelin::player::Playlist> ControllerImpl::getQueue() const
{
    thread::BlockLock bl(m_mutex);
    return std::static_pointer_cast<zeppelin::player::Playlist>(m_playerQueue.clone());
}

// =====================================================================================================================
zeppelin::player::Controller::Status ControllerImpl::getStatus()
{
    Status s;

    thread::BlockLock bl(m_mutex);

    if (m_playerQueue.isValid())
    {
	s.m_file = m_playerQueue.file();
	m_playerQueue.get(s.m_index);
    }

    s.m_state = m_state;
    s.m_position = m_player->getPosition();
    s.m_volume = m_player->getVolumeFilter().getLevel();

    return s;
}

// =====================================================================================================================
void ControllerImpl::queue(const std::shared_ptr<zeppelin::player::QueueItem>& item)
{
    {
	thread::BlockLock bl(m_mutex);
	m_decoderQueue.add(item);
	m_playerQueue.add(item);
    }

    // send event
    m_listenerProxy.queueChanged();
}

// =====================================================================================================================
void ControllerImpl::remove(const std::vector<int>& index)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Remove>(index));
    m_cond.signal();
}

// =====================================================================================================================
void ControllerImpl::removeAll()
{
    command(REMOVE_ALL);
}

// =====================================================================================================================
void ControllerImpl::play()
{
    command(PLAY);
}

// =====================================================================================================================
void ControllerImpl::pause()
{
    command(PAUSE);
}

// =====================================================================================================================
void ControllerImpl::stop()
{
    command(STOP);
}

// =====================================================================================================================
void ControllerImpl::seek(off_t seconds)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Seek>(seconds));
    m_cond.signal();
}

// =====================================================================================================================
void ControllerImpl::prev()
{
    command(PREV);
}

// =====================================================================================================================
void ControllerImpl::next()
{
    command(NEXT);
}

// =====================================================================================================================
void ControllerImpl::goTo(const std::vector<int>& index)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<GoTo>(index));
    m_cond.signal();
}

// =====================================================================================================================
int ControllerImpl::getVolume() const
{
    thread::BlockLock bl(m_mutex);
    return m_player->getVolumeFilter().getLevel();
}

// =====================================================================================================================
void ControllerImpl::setVolume(int level)
{
    {
	thread::BlockLock bl(m_mutex);
	m_player->getVolumeFilter().setLevel(level);
    }

    // send event
    m_listenerProxy.volumeChanged();
}

// =====================================================================================================================
void ControllerImpl::command(Command cmd)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(cmd));
    m_cond.signal();
}

// =====================================================================================================================
void ControllerImpl::run()
{
    while (1)
    {
	m_mutex.lock();

	while (m_commands.empty())
	    m_cond.wait(m_mutex);

	processCommands();

	m_mutex.unlock();
    }
}

// =====================================================================================================================
void ControllerImpl::processCommands()
{
    while (!m_commands.empty())
    {
	std::shared_ptr<CmdBase> cmd = m_commands.front();
	m_commands.pop_front();

	switch (cmd->m_cmd)
	{
	    case PLAY :
		LOG("controller: play");

		if (m_state == PLAYING)
		    break;

		// reset both decoder and player index to the start of the queue if we are in an undefined state
		if (!m_decoderQueue.isValid())
		{
		    m_decoderQueue.reset(zeppelin::player::QueueItem::FIRST);
		    m_playerQueue.reset(zeppelin::player::QueueItem::FIRST);
		}

		// do nothing in case of the queue is still invalid
		if (!m_decoderQueue.isValid())
		    break;

		// a new song was just loaded, send an event
		m_listenerProxy.songChanged();

		// initialize the decoder if it has no input
		if (!m_decoderInitialized)
		    setDecoderInput();

		if (m_decoderInitialized)
		{
		    startPlayback();
		    m_state = PLAYING;

		    // send event
		    m_listenerProxy.started();
		}

		break;

	    case PAUSE :
		LOG("controller: pause");

		if (m_state != PLAYING)
		    break;

		m_player->pausePlayback();

		m_state = PAUSED;

		// send event
		m_listenerProxy.paused();

		break;

	    case SEEK :
	    {
		Seek& s = static_cast<Seek&>(*cmd);

		LOG("controller: seek " << s.m_seconds);

		// seeking is only allowed in playing and paused states
		if (m_state != PLAYING && m_state != PAUSED)
		    break;

		if (m_state == PLAYING)
		    stopPlayback();

		// set decoder queue index to the same as the player
		setDecoderToPlayerIndex();
		// load the file into the decoder
		setDecoderInput();

		// seek to the given position
		m_decoder->seek(s.m_seconds);
		m_player->seek(s.m_seconds);

		// send event
		m_listenerProxy.positionChanged();

		if (m_state == PLAYING)
		    startPlayback();

		break;
	    }

	    case PREV :
	    case NEXT :
	    case GOTO :
	    {
		LOG("controller: prev/next/goto (" << cmd->m_cmd << ")");

		if (m_state == PLAYING || m_state == PAUSED)
		{
		    stopPlayback();
		    invalidateDecoder();
		}

		if (cmd->m_cmd == PREV)
		{
		    m_playerQueue.prev();
		}
		else if (cmd->m_cmd == NEXT)
		{
		    m_playerQueue.next();
		}
		else if (cmd->m_cmd == GOTO)
		{
		    GoTo& g = static_cast<GoTo&>(*cmd);

		    std::ostringstream ss;
		    for (int i : g.m_index)
			ss << "," << i;
		    LOG("controller: goto " << ss.str().substr(1));

		    m_playerQueue.set(g.m_index);
		}

		// set the decoder to the same position
		setDecoderToPlayerIndex();

		// send event
		m_listenerProxy.songChanged();

		// resume playback if it was running before
		if (m_state == PLAYING)
		{
		    // load the new input into the decoder
		    setDecoderInput();

		    if (m_decoderInitialized)
		    {
			// decoder was initialized succesfully, start playing
			startPlayback();
		    }
		    else
		    {
			// unable to initialize the decoder
			m_state = STOPPED;
			// send event
			m_listenerProxy.stopped();
		    }
		}

		break;
	    }

	    case REMOVE :
	    {
		bool removingCurrent = false;
		Remove& rem = static_cast<Remove&>(*cmd);

		std::ostringstream ss;
		for (int i : rem.m_index)
		    ss << "," << i;
		LOG("controller: remove " << ss.str().substr(1));

		// check whether we want to delete a subtree that contains the currently played song
		if (m_playerQueue.isValid())
		{
		    std::vector<int> it;
		    m_playerQueue.get(it);

		    removingCurrent = true;

		    for (size_t i = 0; i < std::min(rem.m_index.size(), it.size()); ++i)
		    {
			if (rem.m_index[i] != it[i])
			{
			    removingCurrent = false;
			    break;
			}
		    }
		}

		if (removingCurrent)
		{
		    if (m_state == PLAYING || m_state == PAUSED)
		    {
			// stop the decoder and the player because we are removing the currently played song
			stopPlayback();
		    }

		    invalidateDecoder();
		}

		// remove the selected subtree from the queue
		std::vector<int> index = rem.m_index;
		m_decoderQueue.remove(index);
		index = rem.m_index;
		m_playerQueue.remove(index);

		// send events
		m_listenerProxy.queueChanged();
		m_listenerProxy.songChanged();

		if (removingCurrent)
		{
		    // re-initialize the decoder
		    setDecoderInput();

		    if (m_state == PLAYING)
		    {
			if (m_decoderInitialized)
			    startPlayback();
			else
			{
			    m_state = STOPPED;

			    // send event
			    m_listenerProxy.stopped();
			}
		    }
		}

		break;
	    }

	    case REMOVE_ALL :
		LOG("controller: remove-all");

		if (m_state == PLAYING || m_state == PAUSED)
		{
		    // stop playback
		    stopPlayback();
		    // invalidate the decoder
		    invalidateDecoder();
		    // update state
		    m_state = STOPPED;
		    // send event
		    m_listenerProxy.stopped();
		}

		m_decoderQueue.clear();
		m_playerQueue.clear();

		// send events
		m_listenerProxy.queueChanged();
		m_listenerProxy.songChanged();

		break;

	    case STOP :
	    {
		LOG("controller: stop");

		if (m_state != PLAYING && m_state != PAUSED)
		    break;

		// stop both the decoder and the player threads
		stopPlayback();

		// reset the decoder index to the currently played song
		setDecoderToPlayerIndex();
		invalidateDecoder();

		m_state = STOPPED;

		// send event
		m_listenerProxy.stopped();

		break;
	    }

	    case DECODER_FINISHED :
		LOG("controller: decoder finished");

		// jump to the next file
		if (!m_decoderQueue.next())
		{
		    invalidateDecoder();
		    break;
		}

		setDecoderInput();

		if (m_decoderInitialized)
		    m_decoder->startDecoding();

		break;

	    case SONG_FINISHED :
		LOG("controller: song finished");

		// step to the next song
		if (!m_playerQueue.next())
		{
		    m_state = STOPPED;

		    // send event
		    m_listenerProxy.stopped();
		}
		else
		{
		    // send event
		    m_listenerProxy.songChanged();
		}

		break;
	}
    }
}

// =====================================================================================================================
void ControllerImpl::startPlayback()
{
    m_decoder->startDecoding();
    m_player->startPlayback();
}

// =====================================================================================================================
void ControllerImpl::stopPlayback()
{
    // stop the player first because it could send NOTIFY messages to the decoder causing stopDecoding() to never
    // return because the decoder would get new commans from the player again and again
    m_player->stopPlayback();
    m_decoder->stopDecoding();
}

// =====================================================================================================================
void ControllerImpl::setDecoderInput()
{
    while (m_decoderQueue.isValid())
    {
	const zeppelin::library::File& file = *m_decoderQueue.file();

	// open the file
	std::shared_ptr<codec::BaseCodec> input = open(file.m_path + "/" + file.m_name);

	if (!input)
	{
	    // try the next one if we were unable to open
	    if (!m_decoderQueue.next())
		break;

	    continue;
	}

	LOG("controller: playing: " << file.m_path << "/" << file.m_name);

	m_decoder->setInput(input);
	m_decoderInitialized = true;

	return;
    }

    invalidateDecoder();
}

// =====================================================================================================================
void ControllerImpl::invalidateDecoder()
{
    if (m_decoderInitialized)
	m_decoder->setInput(nullptr);

    m_decoderInitialized = false;
}

// =====================================================================================================================
void ControllerImpl::setDecoderToPlayerIndex()
{
    std::vector<int> it;

    m_playerQueue.get(it);
    m_decoderQueue.set(it);
}

// =====================================================================================================================
std::shared_ptr<codec::BaseCodec> ControllerImpl::open(const std::string& file)
{
    // create the codec instance
    std::shared_ptr<codec::BaseCodec> input = m_codecManager.create(file);

    if (!input)
	return nullptr;

    // open the file
    try
    {
	input->open();
    }
    catch (const codec::CodecException& e)
    {
	LOG("controller: unable to open " << file << ": " << e.what());
	return nullptr;
    }

    return input;
}
