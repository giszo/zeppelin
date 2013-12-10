#include "musiclibrary.h"

#include <iostream>

using library::MusicLibrary;
using library::ScanDirectory;

// =====================================================================================================================
MusicLibrary::MusicLibrary(Storage& storage)
    : m_storage(storage)
{
    // start the thread of the worker
    m_worker.start();
}

// =====================================================================================================================
library::Storage& MusicLibrary::getStorage()
{
    return m_storage;
}

// =====================================================================================================================
void MusicLibrary::scanDirectory(const std::string& path)
{
    addWork(std::make_shared<ScanDirectory>(path, *this));
}

// =====================================================================================================================
void MusicLibrary::addWork(const std::shared_ptr<BaseWork>& work)
{
    m_worker.add(work);
}

// =====================================================================================================================
void MusicLibrary::directoryFound(const std::string& path)
{
    // recursively scan sub-directories
    scanDirectory(path);
}

// =====================================================================================================================
void MusicLibrary::musicFound(const std::string& path, const std::string& name)
{
    // add the new file to the library
    File file(-1, path, name);
    m_storage.addFile(file);
}
