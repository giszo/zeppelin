#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"
#include "fifo.h"

#include <zeppelin/player/controller.h>
#include <zeppelin/player/queue.h>

#include <thread/thread.h>
#include <thread/condition.h>
#include <filter/volume.h>

namespace player
{

class ControllerImpl : public zeppelin::player::Controller,
		       public thread::Thread
{
    public:
	/// availables commands for the controller
	enum Command
	{
	    PLAY,
	    PAUSE,
	    STOP,
	    SEEK,
	    PREV,
	    NEXT,
	    GOTO,
	    REMOVE,
	    REMOVE_ALL,
	    // sent by the player thread once all samples of the current track have been written to the output
	    SONG_FINISHED,
	    // sent by the decoder thread when the decoding of the current file has been finished
	    DECODER_FINISHED
	};

	ControllerImpl(const config::Config& config);

	/// returns the current play queue
	std::shared_ptr<zeppelin::player::Playlist> getQueue() const;

	/// returns the current status of the player
	Status getStatus();

	/// puts a new file onto the playback queue
	void queue(const std::shared_ptr<zeppelin::library::File>& file);
	/// puts a new directory onto the playback queue
	void queue(const std::shared_ptr<zeppelin::library::Directory>& directory,
		   const std::vector<std::shared_ptr<zeppelin::library::File>>& files);
	/// puts a new album onto the playback queue
	void queue(const std::shared_ptr<zeppelin::library::Album>& album,
		   const std::vector<std::shared_ptr<zeppelin::library::File>>& files);
	/// removes the referenced part of the queue
	void remove(const std::vector<int>& index);
	/// removes all members of the queue
	void removeAll();

	void play();
	void pause();
	void stop();
	void seek(off_t seconds);
	void prev();
	void next();
	void goTo(const std::vector<int>& index);

	/// returns the current volume level
	int getVolume() const;
	/// sets the volume level (level must be between 0 and 100)
	void setVolume(int level);
	/// increases volume level
	void incVolume();
	/// decreases volume level
	void decVolume();

	void command(Command cmd);

	/// the mainloop of the controller
	void run() override;

    private:
	void startPlayback();
	void stopPlayback();

	// loads the selected file from the decoder queue into the decoder
	void setDecoderInput();
	// invalidates the decoder by clearing its file
	void invalidateDecoder();
	// sets the decoder queue index to the same position as the player queue
	void setDecoderToPlayerIndex();

	std::shared_ptr<codec::BaseCodec> openFile(const zeppelin::library::File& file);

    private:
	/// the state of the player
	State m_state;

	zeppelin::player::Playlist m_decoderQueue;
	bool m_decoderInitialized;

	zeppelin::player::Playlist m_playerQueue;

	struct CmdBase
	{
	    CmdBase(Command cmd) : m_cmd(cmd) {}
	    Command m_cmd;
	};

	struct Seek : public CmdBase
	{
	    Seek(off_t seconds) : CmdBase(SEEK), m_seconds(seconds) {}
	    off_t m_seconds;
	};

	struct GoTo : public CmdBase
	{
	    GoTo(const std::vector<int>& index) : CmdBase(GOTO), m_index(index) {}
	    std::vector<int> m_index;
	};

	struct Remove : public CmdBase
	{
	    Remove(const std::vector<int>& index) : CmdBase(REMOVE), m_index(index) {}
	    std::vector<int> m_index;
	};

	/// controller commands
	std::deque<std::shared_ptr<CmdBase>> m_commands;

	/// fifo for decoder and player threads
	Fifo m_fifo;

	/// input decoder thread filling the sample buffer
	std::unique_ptr<Decoder> m_decoder;

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
