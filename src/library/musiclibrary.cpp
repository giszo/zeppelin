#include "musiclibrary.h"

#include <zeppelin/library/storage.h>

using library::MusicLibraryImpl;

// =====================================================================================================================
MusicLibraryImpl::MusicLibraryImpl(const codec::CodecManager& codecManager,
				   zeppelin::library::Storage& storage,
				   const config::Library& config)
    : m_roots(config.m_roots),
      m_scanner(codecManager, storage, *this),
      m_metaParser(codecManager, storage),
      m_storage(storage)
{
    m_scanner.start();
    m_metaParser.start();
}

// =====================================================================================================================
zeppelin::library::Storage& MusicLibraryImpl::getStorage()
{
    return m_storage;
}

// =====================================================================================================================
void MusicLibraryImpl::scan()
{
    // scan all of the configured root directories
    for (const auto& r : m_roots)
	m_scanner.add(r);

    m_scanner.scan();
}

// =====================================================================================================================
void MusicLibraryImpl::scanningStarted()
{
    // Clear mark from all of the files before starting directory scanning. Mark will be put on existing files so we
    // can delete the non-marked files at the end of the scanning to remove deleted files from the library.
    m_storage.clearMark();
}

// =====================================================================================================================
void MusicLibraryImpl::scanningFinished()
{
    // Delete non-marked files from the library
    m_storage.deleteNonMarked();
}

// =====================================================================================================================
void MusicLibraryImpl::musicFound(const std::shared_ptr<zeppelin::library::File>& file)
{
    // add the file into the library and start metadata parsing if it is a new one
    if (m_storage.addFile(*file))
	m_metaParser.add(file);
}
