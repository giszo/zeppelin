#ifndef LIBRARY_SCANDIRECTORY_H_INCLUDED
#define LIBRARY_SCANDIRECTORY_H_INCLUDED

#include "worker.h"

#include <string>

namespace library
{

class DirectoryScannerListener
{
    public:
	virtual ~DirectoryScannerListener()
	{}

	virtual void directoryFound(const std::string& path) = 0;
	virtual void musicFound(const std::string& path, const std::string& name) = 0;
};

class ScanDirectory : public BaseWork
{
    public:
	ScanDirectory(const std::string& path, DirectoryScannerListener& listener);

	void run() override;

    private:
	bool isMediaFile(const std::string& name);

    private:
	std::string m_path;

	DirectoryScannerListener& m_listener;

	static const std::string s_mediaExtensions[];
};

}

#endif
