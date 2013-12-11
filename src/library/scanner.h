#ifndef LIBRARY_SCANNER_H_INCLUDED
#define LIBRARY_SCANNER_H_INCLUDED

#include <thread/thread.h>
#include <thread/mutex.h>
#include <thread/condition.h>

#include <string>
#include <deque>

namespace library
{

class ScannerListener
{
    public:
	virtual ~ScannerListener()
	{}

	virtual void musicFound(const std::string& path, const std::string& name) = 0;
};

class Scanner : public thread::Thread
{
    public:
	Scanner(ScannerListener& listener);

	void add(const std::string& path);

	void run() override;

    private:
	void scanDirectory(const std::string& path);

	bool isMediaFile(const std::string& name);

    private:
	std::deque<std::string> m_paths;

	thread::Mutex m_mutex;
	thread::Condition m_cond;

	ScannerListener& m_listener;

	static const std::string s_mediaExtensions[];
};

}

#endif
