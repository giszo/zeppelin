#ifndef ZEPPELIN_LIBRARY_MUSICLIBRARY_H_INCLUDED
#define ZEPPELIN_LIBRARY_MUSICLIBRARY_H_INCLUDED

namespace zeppelin
{
namespace library
{

class Storage;

class MusicLibrary
{
    public:
	struct Status
	{
	    bool m_scannerRunning;
	};

	virtual ~MusicLibrary()
	{}

	// returns the current status of the library
	virtual Status getStatus() = 0;

	// returns the storage engine of the library
	virtual Storage& getStorage() = 0;

	// initiates a new scanning on the library
	virtual void scan() = 0;
};

}
}

#endif
