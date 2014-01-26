#ifndef ZEPPELIN_CONTROLLER_H_INCLUDED
#define ZEPPELIN_CONTROLLER_H_INCLUDED

#include <memory>
#include <vector>

namespace zeppelin
{
namespace library
{
class File;
class Directory;
class Album;
}

namespace player
{

class Playlist;

class Controller
{
    public:
	enum State
	{
	    STOPPED,
	    PLAYING,
	    PAUSED
	};

	struct Status
	{
	    // the currently played file
	    std::shared_ptr<zeppelin::library::File> m_file;
	    // the tree index of the currently played file
	    std::vector<int> m_index;
	    // the state of the player
	    State m_state;
	    // position inside the current track in seconds
	    unsigned m_position;
	    // volume level (0 - 100)
	    int m_volume;
	};

	virtual ~Controller()
	{}

	/// returns the current play queue
	virtual std::shared_ptr<zeppelin::player::Playlist> getQueue() const = 0;

	/// returns the current status of the player
	virtual Status getStatus() = 0;

	/// puts a new file onto the playback queue
	virtual void queue(const std::shared_ptr<zeppelin::library::File>& file) = 0;
	/// puts a new directory onto the playback queue
	virtual void queue(const std::shared_ptr<zeppelin::library::Directory>& directory,
			   const std::vector<std::shared_ptr<zeppelin::library::File>>& files) = 0;
	/// puts a new album onto the playback queue
	virtual void queue(const std::shared_ptr<zeppelin::library::Album>& album,
			   const std::vector<std::shared_ptr<zeppelin::library::File>>& files) = 0;
	/// removes the referenced part of the queue
	virtual void remove(const std::vector<int>& index) = 0;
	/// removes all members of the queue
	virtual void removeAll() = 0;

	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void stop() = 0;
	virtual void seek(off_t seconds) = 0;
	virtual void prev() = 0;
	virtual void next() = 0;
	virtual void goTo(const std::vector<int>& index) = 0;

	/// returns the current volume level
	virtual int getVolume() const = 0;
	/// sets the volume level (level must be between 0 and 100)
	virtual void setVolume(int level) = 0;
};

}
}

#endif
