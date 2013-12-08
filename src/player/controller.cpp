#include "controller.h"

#include <output/alsa.h>
#include <codec/mp3.h>
#include <thread/blocklock.h>

#include <iostream>

using player::Controller;

// =====================================================================================================================
Controller::Controller()
    : m_samples(1 * 1024 * 1024 /* 1Mb for now... */),
      m_decoder(m_samples),
      m_player(m_samples)
{
    // start the decoder thread
    m_decoder.start();
    // start the player thread
    m_player.start();
}

// =====================================================================================================================
void Controller::queue(const library::File& file)
{
    thread::BlockLock bl(m_mutex);
    m_queue.push_back(file);
}

// =====================================================================================================================
void Controller::play()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(PLAY);
    m_cond.signal();
}

// =====================================================================================================================
void Controller::stop()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(STOP);
    m_cond.signal();
}

// =====================================================================================================================
void Controller::run()
{
    std::shared_ptr<codec::BaseCodec> input;
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
		if (!input)
		{
		    // make sure we have something in the queue to play ...
		    if (m_queue.empty())
			break;

		    library::File& file = m_queue.front();

		    // open the file
		    input.reset(new codec::Mp3());
		    input->open(file.m_path + "/" + file.m_name);

		    // set the input for the decoder thread
		    m_decoder.setInput(input);
		}

		// tell the decoder that it's time to work ... :]
		m_decoder.work();

		// ... start the player as well
		m_player.play();

		break;

	    case STOP :
		// stop both the decoder and the player thread
		m_decoder.suspend();
		m_player.stop();
		break;
	}

	m_mutex.unlock();
    }
}
