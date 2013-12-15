#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"
#include "fifo.h"

#include <library/musiclibrary.h>
#include <thread/condition.h>
#include <filter/volume.h>

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
    // volume level (0 - 100)
    int m_volume;
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
	    GOTO,
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
	void goTo(int index);

	/// sets the volume level (level must be between 0 and 100)
	void setVolume(int level);
	/// increases volume level
	void incVolume();
	/// decreases volume level
	void decVolume();

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

	struct CmdBase
	{
	    CmdBase(Command cmd) : m_cmd(cmd) {}
	    Command m_cmd;
	};

	struct GoTo : public CmdBase
	{
	    GoTo(int index) : CmdBase(GOTO), m_index(index) {}
	    int m_index;
	};

	/// controller commands
	std::deque<std::shared_ptr<CmdBase>> m_commands;

	/// fifo for decoder and player threads
	Fifo m_fifo;

	/// input decoder thread filling the sample buffer
	Decoder m_decoder;

	/// player thread putting decoded samples to the output device
	std::unique_ptr<Player> m_player;

	/// current volume level (between 0 and 100)
	int m_volumeLevel;
	/// volume adjuster filter
	std::shared_ptr<filter::Volume> m_volumeAdj;

	thread::Mutex m_mutex;
	thread::Condition m_cond;
};

}

#endif
