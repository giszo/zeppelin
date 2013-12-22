#include "controller.h"

#include <output/alsa.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Controller;

// =====================================================================================================================
Controller::Controller()
    : m_state(STOPPED),
      m_decoderInitialized(false),
      m_fifo(4 * 1024 /* 4kB for now */),
      m_decoder(m_fifo, *this)
{
    // prepare the output
    std::shared_ptr<output::BaseOutput> output = std::make_shared<output::AlsaOutput>();
    output->setup(44100, 2);

    // prepare player thread
    m_player.reset(new Player(output, m_fifo, *this));

    // create and register volume adjuster filter
    m_volumeAdj.reset(new filter::Volume());
    setVolume(100 /* max */);
    m_decoder.addFilter(m_volumeAdj);

    // start decoder and player threads
    m_decoder.start();
    m_player->start();
}

// =====================================================================================================================
std::shared_ptr<player::Playlist> Controller::getQueue() const
{
    thread::BlockLock bl(m_mutex);
    return std::static_pointer_cast<Playlist>(m_playerQueue.clone());
}

// =====================================================================================================================
auto Controller::getStatus() -> Status
{
    Status s;

    thread::BlockLock bl(m_mutex);

    if (m_playerQueue.isValid())
	s.m_file = m_playerQueue.file();
    s.m_state = m_state;
    s.m_position = m_player->getPosition();
    s.m_volume = m_volumeLevel;

    return s;
}

// =====================================================================================================================
void Controller::queue(const std::shared_ptr<library::File>& file)
{
    thread::BlockLock bl(m_mutex);
    m_decoderQueue.add(file);
    m_playerQueue.add(file);
}

// =====================================================================================================================
void Controller::play()
{
    std::cout << "controller: play" << std::endl;
    command(PLAY);
}

// =====================================================================================================================
void Controller::pause()
{
    std::cout << "controller: pause" << std::endl;
    command(PAUSE);
}

// =====================================================================================================================
void Controller::stop()
{
    std::cout << "controller: stop" << std::endl;
    command(STOP);
}

// =====================================================================================================================
void Controller::prev()
{
    std::cout << "controller: prev" << std::endl;
    command(PREV);
}

// =====================================================================================================================
void Controller::next()
{
    std::cout << "controller: next" << std::endl;
    command(NEXT);
}

// =====================================================================================================================
void Controller::goTo(const std::vector<int>& index)
{
    std::cout << "controller: goto" << std::endl;
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<GoTo>(index));
    m_cond.signal();
}

// =====================================================================================================================
void Controller::setVolume(int level)
{
    // make sure volume level is valid
    if (level < 0 || level > 100)
	return;

    thread::BlockLock bl(m_mutex);

    m_volumeLevel = level;
    m_volumeAdj->setLevel(level / 100.0f);
}

// =====================================================================================================================
void Controller::incVolume()
{
    thread::BlockLock bl(m_mutex);

    if (m_volumeLevel == 100)
	return;

    ++m_volumeLevel;
    m_volumeAdj->setLevel(m_volumeLevel / 100.0f);
}

// =====================================================================================================================
void Controller::decVolume()
{
    thread::BlockLock bl(m_mutex);

    if (m_volumeLevel == 0)
	return;

    --m_volumeLevel;
    m_volumeAdj->setLevel(m_volumeLevel / 100.0f);
}

// =====================================================================================================================
void Controller::command(Command cmd)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<CmdBase>(cmd));
    m_cond.signal();
}

// =====================================================================================================================
void Controller::run()
{
    while (1)
    {
	m_mutex.lock();

	while (m_commands.empty())
	    m_cond.wait(m_mutex);

	std::shared_ptr<CmdBase> cmd = m_commands.front();
	m_commands.pop_front();

	std::cout << "controller: cmd=" << cmd->m_cmd << ", state=" << m_state << std::endl;

	switch (cmd->m_cmd)
	{
	    case PLAY :
		if (m_state == PLAYING)
		    break;

		// reset both decoder and player index to the start of the queue if we are in an undefined state
		if (!m_decoderQueue.isValid())
		{
		    m_decoderQueue.reset();
		    m_playerQueue.reset();
		}

		// initialize the decoder if it has no input
		if (!m_decoderInitialized)
		    setDecoderInput();

		if (m_decoderInitialized)
		{
		    m_decoder.startDecoding();
		    m_player->startPlayback();

		    m_state = PLAYING;
		}

		break;

	    case PAUSE :
		if (m_state != PLAYING)
		    break;

		m_player->pausePlayback();

		m_state = PAUSED;

		break;

	    case PREV :
	    case NEXT :
	    case GOTO :
	    {
		if (m_state == PLAYING || m_state == PAUSED)
		{
		    m_decoder.stopDecoding();
		    m_player->stopPlayback();
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
		    m_playerQueue.set(g.m_index);
		}

		// set the decoder to the same position
		std::vector<int> it;
		m_playerQueue.get(it);
		m_decoderQueue.set(it);

		// load the file into the decoder
		setDecoderInput();

		if (m_decoderInitialized && m_state == PLAYING)
		{
		    m_decoder.startDecoding();
		    m_player->startPlayback();
		}

		break;
	    }

	    case STOP :
	    {
		if (m_state != PLAYING && m_state != PAUSED)
		    break;

		// stop both the decoder and the player threads
		m_decoder.stopDecoding();
		m_player->stopPlayback();

		// reset the decoder index to the currently played song
		std::vector<int> it;
		m_playerQueue.get(it);
		m_decoderQueue.set(it);
		m_decoderInitialized = false;
		m_decoder.setInput(nullptr);

		m_state = STOPPED;

		break;
	    }

	    case DECODER_FINISHED :
		// jump to the next file
		m_decoderQueue.next();

		setDecoderInput();

		if (m_decoderInitialized)
		    m_decoder.startDecoding();

		break;

	    case SONG_FINISHED :
		std::cout << "song finished" << std::endl;

		// step to the next song
		m_playerQueue.next();

		if (!m_playerQueue.isValid())
		    m_state = STOPPED;

		break;
	}

	m_mutex.unlock();
    }
}

// =====================================================================================================================
void Controller::setDecoderInput()
{
    while (m_decoderQueue.isValid())
    {
	const library::File& file = *m_decoderQueue.file();

	// open the file
	std::shared_ptr<codec::BaseCodec> input = codec::BaseCodec::openFile(file.m_path + "/" + file.m_name);

	// try the next one if we were unable to open
	if (!input)
	{
	    m_decoderQueue.next();
	    continue;
	}

	std::cout << "Playing file: " << file.m_path << "/" << file.m_name << std::endl;

	m_decoder.setInput(input);
	m_decoderInitialized = true;

	return;
    }

    m_decoder.setInput(nullptr);
    m_decoderInitialized = false;
}
