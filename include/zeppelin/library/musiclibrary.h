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
	virtual ~MusicLibrary()
	{}

	virtual Storage& getStorage() = 0;

	virtual void scan() = 0;
};

}
}

#endif
