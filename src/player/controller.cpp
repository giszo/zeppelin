#include "controller.h"

#include <output/alsa.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Controller;

// =====================================================================================================================
Controller::Controller()
    : m_samples(1 * 1024 * 1024 /* 1Mb for now... */),
      m_decoder(m_samples, *this),
      m_player(m_samples, *this)
{
    // start the decoder thread
    m_decoder.start();
    // start the player thread
    m_player.start();
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> Controller::getQueue()
{
    std::vector<std::shared_ptr<library::File>> q;

    {
	thread::BlockLock bl(m_mutex);

	for (const auto& file : m_queue)
	    q.push_back(file);
    }

    return q;
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
void Controller::stop()
{
    std::cout << "controller: stop" << std::endl;
    command(STOP);
}

// =====================================================================================================================
void Controller::command(Command cmd)
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(cmd);
    m_cond.signal();
}

// =====================================================================================================================
void Controller::run()
{
    std::shared_ptr<output::BaseOutput> output;

    // prepare the output
    output.reset(new output::AlsaOutput());
    output->setup(44100, 2);

    m_player.setOutput(output);

    while (1)
    {
	m_mutex.lock();

	while (m_commands.empty())
	    m_cond.wait(m_mutex);

	Command cmd = m_commands.front();
	m_commands.pop_front();

	std::cout << "controller: cmd=" << cmd << std::endl;

	switch (cmd)
	{
	    case PLAY :
		startPlayback();
		break;

	    case STOP :
		// stop both the decoder and the player thread
		m_decoder.suspend();
		m_player.stop();
		break;

	    case PLAYER_DONE :
		// force startPlayback() to get the next file from the queue
		m_currentFile.reset();
		startPlayback();
		break;

	    case DECODER_WORKING :
		// now we can start the player because the decoder started to fill the input buffer
		m_player.play();
		break;
	}

	m_mutex.unlock();
    }
}

// =====================================================================================================================
void Controller::startPlayback()
{
    // select the next file to play if we have none already
    while (!m_queue.empty() && !m_currentFile)
    {
	// get the next one
	m_currentFile = m_queue.front();
	m_queue.pop_front();

	// open the file
	m_input = codec::BaseCodec::openFile(m_currentFile->m_path + "/" + m_currentFile->m_name);

	if (!m_input)
	{
	    // try the next one if we were not able to open this file
	    m_currentFile.reset();
	    continue;
	}

	std::cout << "Playing file: " << m_currentFile->m_path << "/" << m_currentFile->m_name << std::endl;

	// set the input for the decoder thread
	m_decoder.setInput(m_input);
    }

    // can't start playback without a file
    if (!m_currentFile)
	return;

    // start the decoder thread here... the player will be started later on once the decoder notifies us
    // with DECODER_WORKING
    m_decoder.work();
}
