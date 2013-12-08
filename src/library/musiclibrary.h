#ifndef LIBRARY_MUSICLIBRARY_H_INCLUDED
#define LIBRARY_MUSICLIBRARY_H_INCLUDED

#include "worker.h"
#include "scandirectory.h"

#include <vector>
#include <stdexcept>

#include <sqlite3.h>
#include <pthread.h>

namespace library
{

struct File
{
    File(int id, const std::string& path, const std::string& name)
	: m_id(id), m_path(path), m_name(name)
    {}

    int m_id;
    std::string m_path;
    std::string m_name;
};

class FileNotFoundException : public std::runtime_error
{
    public:
	FileNotFoundException(const std::string& error)
	    : runtime_error(error)
	{}
};

class MusicLibrary : public DirectoryScannerListener
{
    public:
	MusicLibrary();
	~MusicLibrary();

	void open();

	std::shared_ptr<File> getFile(int id);
	std::vector<File> getFileList();

	void scanDirectory(const std::string& path);

	void directoryFound(const std::string& path) override;
	void musicFound(const std::string& path, const std::string& name) override;

    private:
	Worker m_worker;

	// the database to store the music library
	sqlite3* m_db;

	// prepared statements for the database
	sqlite3_stmt* m_newfile;
	sqlite3_stmt* m_getfile;
	sqlite3_stmt* m_listfiles;

	// mutex for the music database
	pthread_mutex_t m_lock;
};

}

#endif
