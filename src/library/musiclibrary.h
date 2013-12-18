#ifndef LIBRARY_MUSICLIBRARY_H_INCLUDED
#define LIBRARY_MUSICLIBRARY_H_INCLUDED

#include "scanner.h"
#include "storage.h"
#include "metaparser.h"

#include <config/config.h>

namespace library
{

class MusicLibrary : public ScannerListener
{
    public:
	MusicLibrary(Storage& storage, const config::Library& config);

	Storage& getStorage();

	void scan();

	void scanningStarted() override;
	void scanningFinished() override;
	void musicFound(const std::string& path, const std::string& name) override;

    private:
	/// configured root directories for searching music files
	std::vector<std::string> m_roots;

	Scanner m_scanner;
	MetaParser m_metaParser;

	/// music library storage
	Storage& m_storage;
};

}

#endif
