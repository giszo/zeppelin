#ifndef LIBRARY_MUSICLIBRARY_H_INCLUDED
#define LIBRARY_MUSICLIBRARY_H_INCLUDED

#include "worker.h"
#include "scandirectory.h"
#include "storage.h"

namespace library
{

class MusicLibrary : public DirectoryScannerListener
{
    public:
	MusicLibrary(Storage& storage);

	Storage& getStorage();

	void scanDirectory(const std::string& path);

	/// puts a new work onto the queue of the music library
	void addWork(const std::shared_ptr<BaseWork>& work);

	void directoryFound(const std::string& path) override;
	void musicFound(const std::string& path, const std::string& name) override;

    private:
	Worker m_worker;

	/// music library storage
	Storage& m_storage;
};

}

#endif
