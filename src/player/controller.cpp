#include "controller.h"

#include <output/alsa.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Controller;

// =====================================================================================================================
Controller::Controller()
    : m_state(STOPPED),
      m_decoderIndex(0),
      m_decoderInitialized(false),
      m_playerIndex(0),
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
std::vector<std::shared_ptr<library::File>> Controller::getQueue()
{
    thread::BlockLock bl(m_mutex);
    return m_queue;
}

// =====================================================================================================================
auto Controller::getStatus() -> Status
{
    Status s;

    thread::BlockLock bl(m_mutex);

    if (isPlayerIndexValid())
	s.m_file = m_queue[m_playerIndex];
    s.m_state = m_state;
    s.m_position = m_player->getPosition();
    s.m_volume = m_volumeLevel;

    return s;
}

// =====================================================================================================================
void Controller::queue(const std::shared_ptr<library::File>& file)
{
    thread::BlockLock bl(m_mutex);
    m_queue.push_back(file);
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
void Controller::goTo(int index)
{
    std::cout << "controller: goto " << index << std::endl;
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

	std::cout << "controller: cmd=" << cmd->m_cmd << std::endl;
	std::cout << "controller: state=" << m_state <<
	    ", decoderIndex=" << m_decoderIndex <<
	    ", playerIndex=" << m_playerIndex << std::endl;

	switch (cmd->m_cmd)
	{
	    case PLAY :
		if (m_state == PLAYING)
		    break;

		// reset both decoder and player index to the start of the queue if we are in an undefined state
		if (!isDecoderIndexValid())
		{
		    m_decoderIndex = 0;
		    m_playerIndex = 0;
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
		if (m_state == PLAYING || m_state == PAUSED)
		{
		    m_decoder.stopDecoding();
		    m_player->stopPlayback();
		}

		if (cmd->m_cmd == PREV)
		{
		    if (m_playerIndex > 0) --m_playerIndex;
		}
		else if (cmd->m_cmd == NEXT)
		{
		    if (!m_queue.empty() && (m_playerIndex < static_cast<int>(m_queue.size() - 1))) ++m_playerIndex;
		}
		else if (cmd->m_cmd == GOTO)
		{
		    const GoTo& g = static_cast<GoTo&>(*cmd);
		    if (g.m_index >= 0 && g.m_index < static_cast<int>(m_queue.size())) m_playerIndex = g.m_index;
		}

		// set the decoder to the same position
		m_decoderIndex = m_playerIndex;

		// load the file into the decoder
		setDecoderInput();

		if (m_decoderInitialized && m_state == PLAYING)
		{
		    m_decoder.startDecoding();
		    m_player->startPlayback();
		}

		break;

	    case STOP :
		if (m_state != PLAYING && m_state != PAUSED)
		    break;

		// stop both the decoder and the player threads
		m_decoder.stopDecoding();
		m_player->stopPlayback();

		// reset the decoder index to the currently played song
		m_decoderIndex = m_playerIndex;
		m_decoderInitialized = false;
		m_decoder.setInput(nullptr);

		m_state = STOPPED;

		break;

	    case DECODER_FINISHED :
		// jump to the next file
		++m_decoderIndex;

		setDecoderInput();

		if (m_decoderInitialized)
		    m_decoder.startDecoding();

		break;

	    case SONG_FINISHED :
		std::cout << "song finished" << std::endl;

		// step to the next song
		++m_playerIndex;

		if (!isPlayerIndexValid())
		    m_state = STOPPED;

		break;
	}

	m_mutex.unlock();
    }
}

// =====================================================================================================================
bool Controller::isDecoderIndexValid() const
{
    return (m_decoderIndex >= 0 && m_decoderIndex < static_cast<int>(m_queue.size()));
}

// =====================================================================================================================
bool Controller::isPlayerIndexValid() const
{
    return (m_playerIndex >= 0 && m_playerIndex < static_cast<int>(m_queue.size()));
}

// =====================================================================================================================
void Controller::setDecoderInput()
{
    while (m_decoderIndex < static_cast<int>(m_queue.size()))
    {
	const library::File& file = *m_queue[m_decoderIndex];

	// open the file
	std::shared_ptr<codec::BaseCodec> input = codec::BaseCodec::openFile(file.m_path + "/" + file.m_name);

	// try the next one if we were unable to open
	if (!input)
	{
	    ++m_decoderIndex;
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
