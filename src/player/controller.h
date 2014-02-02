#ifndef PLAYER_CONTORLLER_H_INCLUDED
#define PLAYER_CONTORLLER_H_INCLUDED

#include "decoder.h"
#include "player.h"
#include "fifo.h"
#include "eventlistenerproxy.h"

#include <zeppelin/player/controller.h>
#include <zeppelin/player/queue.h>

#include <thread/thread.h>
#include <thread/condition.h>
#include <filter/volume.h>

namespace codec
{
class CodecManager;
}

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

	static std::shared_ptr<ControllerImpl> create(const codec::CodecManager& codecManager,
						      const std::shared_ptr<Decoder>& decoder,
						      const std::shared_ptr<Player>& player,
						      const config::Config& config);

	void addListener(const std::shared_ptr<zeppelin::player::EventListener>& listener) override;

	/// returns the current play queue
	std::shared_ptr<zeppelin::player::Playlist> getQueue() const;

	/// returns the current status of the player
	Status getStatus();

	/// puts a new item onto the playback queue
	void queue(const std::shared_ptr<zeppelin::player::QueueItem>& item);
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

	void command(Command cmd);

	/// the mainloop of the controller
	void run() override;

    private:
	ControllerImpl(const codec::CodecManager& codecManager,
		       const std::shared_ptr<Decoder>& decoder,
		       const std::shared_ptr<Player>& player,
		       const config::Config& config);

	// called to initialize the controller after m_selfRef is usable
	void init();

	void processCommands();

	void startPlayback();
	void stopPlayback();

	// loads the selected file from the decoder queue into the decoder
	void setDecoderInput();
	// invalidates the decoder by clearing its file
	void invalidateDecoder();
	// sets the decoder queue index to the same position as the player queue
	void setDecoderToPlayerIndex();

	std::shared_ptr<codec::BaseCodec> open(const std::string& file);

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

	/// input decoder thread filling the sample buffer
	std::shared_ptr<Decoder> m_decoder;

	/// player thread putting decoded samples to the output device
	std::shared_ptr<Player> m_player;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	const codec::CodecManager& m_codecManager;

	EventListenerProxy m_listenerProxy;

	std::weak_ptr<ControllerImpl> m_selfRef;
};

}

#endif
