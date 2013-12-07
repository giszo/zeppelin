#ifndef PLAYER_PLAYER_H_INCLUDED
#define PLAYER_PLAYER_H_INCLUDED

#include "queue.h"

#include "../output/baseoutput.h"

namespace player
{

class Player
{
    public:
	Player();

	void queue(const library::File& file);

	void play();
	void stop();

	void run();

    private:
	Queue m_queue;

	std::shared_ptr<output::BaseOutput> m_output;
};

}

#endif
