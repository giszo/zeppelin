#include "scandirectory.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using library::ScanDirectory;

const std::string ScanDirectory::s_mediaExtensions[] = {
    ".mp3"
};

// =====================================================================================================================
ScanDirectory::ScanDirectory(const std::string& path, DirectoryScannerListener& listener)
    : m_path(path),
      m_listener(listener)
{
}

// =====================================================================================================================
void ScanDirectory::run()
{
    // open the directory
    DIR* dir = opendir(m_path.c_str());

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
	std::string path = m_path + "/" + name;

	if (stat(path.c_str(), &st) != 0)
	    continue;

	if (S_ISDIR(st.st_mode))
	    m_listener.directoryFound(path);
	else if (isMediaFile(name))
	    m_listener.musicFound(m_path, name);
    }

    closedir(dir);
}

// =====================================================================================================================
bool ScanDirectory::isMediaFile(const std::string& name)
{
    for (size_t i = 0; i < sizeof(s_mediaExtensions) / sizeof(s_mediaExtensions[0]); ++i)
    {
	const std::string& ext = s_mediaExtensions[i];

	if (name.length() < ext.length())
	    continue;

	if (name.compare(name.length() - ext.length(), ext.length(), ext) == 0)
	    return true;
    }

    return false;
}
