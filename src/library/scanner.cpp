#include "scanner.h"

#include <codec/basecodec.h>
#include <thread/blocklock.h>
#include <utils/stringutils.h>

#include <zeppelin/logger.h>
#include <zeppelin/library/storage.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using library::Scanner;

// =====================================================================================================================
Scanner::Scanner(zeppelin::library::Storage& storage, ScannerListener& listener)
    : m_storage(storage),
      m_listener(listener)
{
}

// =====================================================================================================================
void Scanner::add(const std::string& path)
{
    thread::BlockLock bl(m_mutex);
    m_paths.push_back({-1, path});
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
    std::deque<Directory> paths;

    {
	thread::BlockLock bl(m_mutex);
	paths = m_paths;
	m_paths.clear();
    }

    // get the ID of the root directories
    for (auto& p : paths)
	p.m_id = m_storage.ensureDirectory(p.m_path, -1);

    while (!paths.empty())
    {
	Directory path = paths.front();
	paths.pop_front();

	scanDirectory(path, paths);
    }
}

// =====================================================================================================================
void Scanner::scanDirectory(const Directory& path, std::deque<Directory>& paths)
{
    LOG("Scanning directory: " << path.m_path);

    // open the directory
    DIR* dir = opendir(path.m_path.c_str());

    if (!dir)
    {
	LOG("Unable to open directory: " << path.m_path);
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
	std::string p = path.m_path + "/" + name;

	if (stat(p.c_str(), &st) != 0)
	    continue;

	if (S_ISDIR(st.st_mode))
	{
	    int directoryId = m_storage.ensureDirectory(name, path.m_id);
	    paths.push_back({directoryId, p});
	}
	else if (codec::BaseCodec::isMediaFile(name))
	{
	    zeppelin::library::File file(-1);
	    file.m_directoryId = path.m_id;
	    file.m_path = path.m_path;
	    file.m_name = name;
	    file.m_size = st.st_size;

	    m_listener.musicFound(file);
	}
    }

    closedir(dir);

    LOG("Scanning of " << path.m_path << " finished");
}
