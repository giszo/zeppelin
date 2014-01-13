#include "musiclibrary.h"

using library::MusicLibrary;

// =====================================================================================================================
MusicLibrary::MusicLibrary(Storage& storage, const config::Library& config)
    : m_roots(config.m_roots),
      m_scanner(storage, *this),
      m_metaParser(*this),
      m_storage(storage)
{
    m_scanner.start();
    m_metaParser.start();
}

// =====================================================================================================================
library::Storage& MusicLibrary::getStorage()
{
    return m_storage;
}

// =====================================================================================================================
void MusicLibrary::scan()
{
    // scan all of the configured root directories
    for (const auto& r : m_roots)
	m_scanner.add(r);

    m_scanner.scan();
}

// =====================================================================================================================
void MusicLibrary::scanningStarted()
{
    // Clear mark from all of the files before starting directory scanning. Mark will be put on existing files so we
    // can delete the non-marked files at the end of the scanning to remove deleted files from the library.
    m_storage.clearMark();
}

// =====================================================================================================================
void MusicLibrary::scanningFinished()
{
    // Delete non-marked files from the library
    m_storage.deleteNonMarked();
}

// =====================================================================================================================
void MusicLibrary::musicFound(const File& file)
{
    std::shared_ptr<File> f(new File(file));

    // add the file into the library and start metadata parsing if it is a new one
    if (m_storage.addFile(*f))
	m_metaParser.add(f);
}
