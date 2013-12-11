#include "scanner.h"

#include <thread/blocklock.h>
#include <utils/stringutils.h>

#include <iostream>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using library::Scanner;

const std::string Scanner::s_mediaExtensions[] = {
    ".mp3"
};

// =====================================================================================================================
Scanner::Scanner(ScannerListener& listener)
    : m_listener(listener)
{
}

// =====================================================================================================================
void Scanner::add(const std::string& path)
{
    thread::BlockLock bl(m_mutex);
    m_paths.push_back(path);
    m_cond.signal();
}

// =====================================================================================================================
void Scanner::run()
{
    while (1)
    {
	std::string path;

	m_mutex.lock();

	while (m_paths.empty())
	    m_cond.wait(m_mutex);

	path = m_paths.front();
	m_paths.pop_front();

	m_mutex.unlock();

	scanDirectory(path);
    }
}

// =====================================================================================================================
void Scanner::scanDirectory(const std::string& path)
{
    std::cout << "Scanning directory: " << path << std::endl;

    // open the directory
    DIR* dir = opendir(path.c_str());

    if (!dir)
	return;

    // iterate through directory entries
    struct dirent* ent;

    while ((ent = readdir(dir)) != NULL)
    {
	std::string name = ent->d_name;

	if (name == "." || name == "..")
	    continue;

	struct stat st;
	std::string p = path + "/" + name;

	if (stat(p.c_str(), &st) != 0)
	    continue;

	if (S_ISDIR(st.st_mode))
	{
	    thread::BlockLock bl(m_mutex);
	    m_paths.push_back(p);
	    // NOTE: no need to signal the condition here
	}
	else if (isMediaFile(name))
	    m_listener.musicFound(path, name);
    }

    closedir(dir);
}

// =====================================================================================================================
bool Scanner::isMediaFile(const std::string& name)
{
    for (size_t i = 0; i < sizeof(s_mediaExtensions) / sizeof(s_mediaExtensions[0]); ++i)
    {
	const std::string& ext = s_mediaExtensions[i];

	if (utils::StringUtils::endsWith(name, ext))
	    return true;
    }

    return false;
}
