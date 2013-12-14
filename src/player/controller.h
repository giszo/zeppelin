#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"
#include "fifo.h"

#include <library/musiclibrary.h>
#include <thread/condition.h>

#include <vector>

namespace player
{

enum State
{
    STOPPED,
    PLAYING,
    PAUSED
};

struct Status
{
    // the currently played file
    std::shared_ptr<library::File> m_file;
    // the state of the player
    State m_state;
    // position inside the current track in seconds
    unsigned m_position;
};

class Controller
{
    public:
	/// availables commands for the controller
	enum Command
	{
	    PLAY,
	    PAUSE,
	    STOP,
	    PREV,
	    NEXT,
	    // sent by the player thread once all samples of the current track have been written to the output
	    SONG_FINISHED,
	    // sent by the decoder thread when the decoding of the current file has been finished
	    DECODER_FINISHED
	};

	Controller();

	/// returns the current play queue
	std::vector<std::shared_ptr<library::File>> getQueue();

	/// returns the current status of the player
	Status getStatus();

	/// puts a new file onto the playback queue
	void queue(const std::shared_ptr<library::File>& file);

	void play();
	void pause();
	void stop();
	void prev();
	void next();

	void command(Command cmd);

	/// the mainloop of the controller
	void run();

    private:
	bool isDecoderIndexValid() const;
	bool isPlayerIndexValid() const;

	void setDecoderInput();

    private:
	/// queued files for playing
	std::vector<std::shared_ptr<library::File>> m_queue;

	/// the state of the player
	State m_state;

	/// the index of the currently decoded file from the queue
	int m_decoderIndex;
	/// true when the decoder has the current file as input
	bool m_decoderInitialized;

	/// the index of the currently played file from the queue
	int m_playerIndex;

	/// controller commands
	std::deque<Command> m_commands;

	/// fifo for decoder and player threads
	Fifo m_fifo;

	/// input decoder thread filling the sample buffer
	Decoder m_decoder;

	/// player thread putting decoded samples to the output device
	std::unique_ptr<Player> m_player;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
};

}

#endif
