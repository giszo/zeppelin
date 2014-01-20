#include "pidfile.h"

#include <cstring>

#include <unistd.h>
#include <fcntl.h>

using utils::PidFile;

// =====================================================================================================================
PidFile::PidFile(const std::string& path)
    : m_path(path)
{
}

// =====================================================================================================================
PidFile::~PidFile()
{
    unlink(m_path.c_str());
}

// =====================================================================================================================
std::unique_ptr<PidFile> PidFile::create(const std::string& path)
{
    // use O_EXCL to make sure we are creating the file
    int fd = open(path.c_str(), O_CREAT | O_EXCL | O_RDWR, 0644);

    if (fd < 0)
	return nullptr;

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%d", getpid());

    ssize_t ret = write(fd, tmp, strlen(tmp));
    (void)ret;

    close(fd);

    return std::unique_ptr<PidFile>(new PidFile(path));
}
