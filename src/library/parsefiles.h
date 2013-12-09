#ifndef LIBRARY_PARSEFILES_H_INCLUDED
#define LIBRARY_PARSEFILES_H_INCLUDED

#include "worker.h"

namespace library
{

class File;
class MusicLibrary;

/**
 * Parses the metadate of the next N not-yet parsed file from the music library.
 */
class ParseFiles : public BaseWork
{
    public:
	ParseFiles(MusicLibrary& library);

	void run() override;

    private:
	void parse(library::File& file);

    private:
	MusicLibrary& m_library;
};

}

#endif
