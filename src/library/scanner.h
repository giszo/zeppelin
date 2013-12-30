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
	Scanner(ScannerListener& listener);

	void add(const std::string& path);

	// starts directory scanning
	void scan();

	void run() override;

    private:
	void scanDirectories();
	void scanDirectory(const std::string& path, std::deque<std::string>& paths);

	bool isMediaFile(const std::string& name);

    private:
	enum Command
	{
	    SCAN
	};

	// command queue
	std::deque<Command> m_commands;

	// list of paths that will be scanned
	std::deque<std::string> m_paths;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	ScannerListener& m_listener;

	static const std::string s_mediaExtensions[];
};

}

#endif
