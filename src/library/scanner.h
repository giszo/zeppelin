#ifndef LIBRARY_SCANNER_H_INCLUDED
#define LIBRARY_SCANNER_H_INCLUDED

#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/condition.h>

#include <string>
#include <deque>

namespace library
{

struct File;
class Storage;

class ScannerListener
{
    public:
	virtual ~ScannerListener()
	{}

	virtual void scanningStarted() = 0;
	virtual void scanningFinished() = 0;

	virtual void musicFound(const File& file) = 0;
};

class Scanner : public thread::Thread
{
    public:
	Scanner(Storage& storage, ScannerListener& listener);

	void add(const std::string& path);

	// starts directory scanning
	void scan();

	void run() override;

    private:
	void scanDirectories();

	struct Directory
	{
	    // ID of the directory in the database
	    int m_id;
	    // path of the directory
	    std::string m_path;
	};

	void scanDirectory(const Directory& path, std::deque<Directory>& paths);

	bool isMediaFile(const std::string& name);

    private:
	enum Command
	{
	    SCAN
	};

	// command queue
	std::deque<Command> m_commands;

	// list of paths that will be scanned
	std::deque<Directory> m_paths;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	Storage& m_storage;
	ScannerListener& m_listener;

	static const std::string s_mediaExtensions[];
};

}

#endif
