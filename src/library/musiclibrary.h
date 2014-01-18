#ifndef LIBRARY_MUSICLIBRARY_H_INCLUDED
#define LIBRARY_MUSICLIBRARY_H_INCLUDED

#include <zeppelin/library/musiclibrary.h>

#include "scanner.h"
#include "metaparser.h"

#include <config/config.h>

namespace library
{

class MusicLibraryImpl : public zeppelin::library::MusicLibrary,
			 public ScannerListener
{
    public:
	MusicLibraryImpl(zeppelin::library::Storage& storage, const config::Library& config);

	zeppelin::library::Storage& getStorage() override;

	void scan() override;

	void scanningStarted() override;
	void scanningFinished() override;
	void musicFound(const zeppelin::library::File& file) override;

    private:
	/// configured root directories for searching music files
	std::vector<std::string> m_roots;

	Scanner m_scanner;
	MetaParser m_metaParser;

	/// music library storage
	zeppelin::library::Storage& m_storage;
};

}

#endif
