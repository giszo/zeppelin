#ifndef LIBRARY_METAPARSER_H_INCLUDED
#define LIBRARY_METAPARSER_H_INCLUDED

#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/condition.h>

#include <deque>
#include <memory>

namespace library
{

class File;
class MusicLibrary;

class MetaParser : public thread::Thread
{
    public:
	MetaParser(MusicLibrary& library);

	void add(const std::shared_ptr<File>& file);

	void run() override;

    private:
	void parse(File& file);

    private:
	std::deque<std::shared_ptr<File>> m_files;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	MusicLibrary& m_library;
};

}

#endif
