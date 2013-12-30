#include "scanner.h"
#include "storage.h"

#include <thread/blocklock.h>
#include <utils/stringutils.h>
#include <logger.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using library::Scanner;

const std::string Scanner::s_mediaExtensions[] = {
    ".mp3",
    ".flac"
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
}

// =====================================================================================================================
void Scanner::scan()
{
    thread::BlockLock bl(m_mutex);
    m_commands.push_back(SCAN);
    m_cond.signal();
}

// =====================================================================================================================
void Scanner::run()
{
    while (1)
    {
	m_mutex.lock();

	while (m_commands.empty())
	    m_cond.wait(m_mutex);

	Command cmd = m_commands.front();
	m_commands.pop_front();

	m_mutex.unlock();

	switch (cmd)
	{
	    case SCAN :
		m_listener.scanningStarted();
		scanDirectories();
		m_listener.scanningFinished();
		break;
	}
    }
}

// =====================================================================================================================
void Scanner::scanDirectories()
{
    std::deque<std::string> paths;

    {
	thread::BlockLock bl(m_mutex);
	paths = m_paths;
	m_paths.clear();
    }

    while (!paths.empty())
    {
	std::string path = paths.front();
	paths.pop_front();

	scanDirectory(path, paths);
    }
}

// =====================================================================================================================
void Scanner::scanDirectory(const std::string& path, std::deque<std::string>& paths)
{
    LOG("Scanning directory: " << path);

    // open the directory
    DIR* dir = opendir(path.c_str());

    if (!dir)
    {
	LOG("Unable to open directory: " << path);
	return;
    }

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
	    paths.push_back(p);
	else if (isMediaFile(name))
	{
	    File file(-1, path, name, st.st_size);
	    m_listener.musicFound(file);
	}
    }

    closedir(dir);

    LOG("Scanning of " << path << " finished");
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
