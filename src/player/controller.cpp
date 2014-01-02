#include "controller.h"

#include <output/alsa.h>
#include <thread/blocklock.h>
#include <logger.h>

#include <sstream>

using player::Controller;

// =====================================================================================================================
Controller::Controller(const config::Config& config)
    : m_state(STOPPED),
      m_decoderInitialized(false),
      m_fifo(4 * 1024 /* 4kB for now */)
{
    // prepare the output
    std::shared_ptr<output::BaseOutput> output = std::make_shared<output::AlsaOutput>(config);
    output->setup(44100, 2);

    Format fmt = output->getFormat();

    // prepare decoder
    m_decoder.reset(new Decoder(fmt.sizeOfSeconds(10 /* 10 seconds of samples */), fmt, m_fifo, *this, config));
    m_fifo.setNotifyCallback(fmt.sizeOfSeconds(5 /* 5 second limit */), std::bind(&Decoder::notify, m_decoder.get()));

    // prepare decoder - volume filter
    m_volumeAdj.reset(new filter::Volume(config));
    m_volumeAdj->init();
    setVolume(100 /* max */);
    m_decoder->addFilter(m_volumeAdj);

    // prepare player
    m_player.reset(new Player(output, m_fifo, *this));

    // start decoder and player threads
    m_decoder->start();
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
void Controller::queue(const std::shared_ptr<library::Album>& album,
		      const std::vector<std::shared_ptr<library::File>>& files)
{
    thread::BlockLock bl(m_mutex);

    m_decoderQueue.add(album, files);
    m_playerQueue.add(album, files);
}

// =====================================================================================================================
void Controller::remove(const std::vector<int>& index)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<Remove>(index));
    m_cond.signal();
}

// =====================================================================================================================
void Controller::play()
{
    command(PLAY);
}

// =====================================================================================================================
void Controller::pause()
{
    command(PAUSE);
}

// =====================================================================================================================
void Controller::stop()
{
    command(STOP);
}

// =====================================================================================================================
void Controller::prev()
{
    command(PREV);
}

// =====================================================================================================================
void Controller::next()
{
    command(NEXT);
}

// =====================================================================================================================
void Controller::goTo(const std::vector<int>& index)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(std::make_shared<GoTo>(index));
    m_cond.signal();
}

// =====================================================================================================================
int Controller::getVolume() const
{
    thread::BlockLock bl(m_mutex);
    return m_volumeLevel;
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

	switch (cmd->m_cmd)
	{
	    case PLAY :
		LOG("controller: play");

		if (m_state == PLAYING)
		    break;

		// reset both decoder and player index to the start of the queue if we are in an undefined state
		if (!m_decoderQueue.isValid())
		{
		    m_decoderQueue.reset(QueueItem::FIRST);
		    m_playerQueue.reset(QueueItem::FIRST);
		}

		// initialize the decoder if it has no input
		if (!m_decoderInitialized)
		    setDecoderInput();

		if (m_decoderInitialized)
		{
		    startPlayback();
		    m_state = PLAYING;
		}

		break;

	    case PAUSE :
		LOG("controller: pause");

		if (m_state != PLAYING)
		    break;

		m_player->pausePlayback();

		m_state = PAUSED;

		break;

	    case PREV :
	    case NEXT :
	    case GOTO :
	    {
		LOG("controller: prev/next/goto (" << cmd->m_cmd << ")");

		if (m_state == PLAYING || m_state == PAUSED)
		{
		    stopPlayback();
		    m_decoderInitialized = false;
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
		    assert(rem.m_index.size() <= it.size());

		    for (size_t i = 0; i < rem.m_index.size(); ++i)
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

		    m_decoder->setInput(nullptr);
		    m_decoderInitialized = false;
		}

		// remove the selected subtree from the queue
		std::vector<int> index = rem.m_index;
		m_decoderQueue.remove(index);
		index = rem.m_index;
		m_playerQueue.remove(index);

		if (removingCurrent)
		{
		    // re-initialize the decoder
		    setDecoderInput();

		    if (m_state == PLAYING)
		    {
			if (m_decoderInitialized)
			    startPlayback();
			else
			    m_state = STOPPED;
		    }
		}

		break;
	    }

	    case STOP :
	    {
		LOG("controller: stop");

		if (m_state != PLAYING && m_state != PAUSED)
		    break;

		// stop both the decoder and the player threads
		stopPlayback();

		// reset the decoder index to the currently played song
		std::vector<int> it;
		m_playerQueue.get(it);
		m_decoderQueue.set(it);
		m_decoderInitialized = false;
		m_decoder->setInput(nullptr);

		m_state = STOPPED;

		break;
	    }

	    case DECODER_FINISHED :
		LOG("controller: decoder finished");

		// jump to the next file
		if (!m_decoderQueue.next())
		{
		    m_decoder->setInput(nullptr);
		    m_decoderInitialized = false;
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
		    m_state = STOPPED;

		break;
	}

	m_mutex.unlock();
    }
}

// =====================================================================================================================
void Controller::startPlayback()
{
    m_decoder->startDecoding();
    m_player->startPlayback();
}

// =====================================================================================================================
void Controller::stopPlayback()
{
    // stop the player first because it could send NOTIFY messages to the decoder causing stopDecoding() to never
    // return because the decoder would get new commans from the player again and again
    m_player->stopPlayback();
    m_decoder->stopDecoding();
}

// =====================================================================================================================
void Controller::setDecoderInput()
{
    while (m_decoderQueue.isValid())
    {
	const library::File& file = *m_decoderQueue.file();

	// open the file
	std::shared_ptr<codec::BaseCodec> input = openFile(file);

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

    m_decoder->setInput(nullptr);
    m_decoderInitialized = false;
}

// =====================================================================================================================
std::shared_ptr<codec::BaseCodec> Controller::openFile(const library::File& file)
{
    std::shared_ptr<codec::BaseCodec> input = codec::BaseCodec::create(file.m_path + "/" + file.m_name);

    if (!input)
	return nullptr;

    try
    {
	input->open();
    }
    catch (const codec::CodecException& e)
    {
	return nullptr;
    }

    return input;
}
