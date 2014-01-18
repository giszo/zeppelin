#ifndef LIBRARY_METAPARSER_H_INCLUDED
#define LIBRARY_METAPARSER_H_INCLUDED

#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/condition.h>

#include <deque>
#include <memory>

namespace zeppelin
{
namespace library
{
struct File;
class Storage;
}
}

namespace library
{

class MusicLibrary;

class MetaParser : public thread::Thread
{
    public:
	MetaParser(zeppelin::library::Storage& storage);

	void add(const std::shared_ptr<zeppelin::library::File>& file);

	void run() override;

    private:
	void parse(zeppelin::library::File& file);

    private:
	std::deque<std::shared_ptr<zeppelin::library::File>> m_files;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	zeppelin::library::Storage& m_storage;
};

}

#endif
