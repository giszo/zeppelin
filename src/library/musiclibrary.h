#ifndef LIBRARY_MUSICLIBRARY_H_INCLUDED
#define LIBRARY_MUSICLIBRARY_H_INCLUDED

#include "worker.h"
#include "scandirectory.h"

#include <thread/mutex.h>

#include <vector>
#include <stdexcept>

#include <sqlite3.h>
#include <pthread.h>

namespace library
{

struct File
{
    File(int id, const std::string& path, const std::string& name, int length)
	: m_id(id), m_path(path), m_name(name), m_length(length)
    {}

    int m_id;
    std::string m_path;
    std::string m_name;

    /// the length of the music file in seconds
    int m_length;
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

	/// retuns the given amount of files at most from the library having no metadata yet
	std::vector<std::shared_ptr<File>> getNewFiles(int amount);

	void scanDirectory(const std::string& path);

	/**
	 * Updates the metadate of the given file.
	 * @param id the ID of the file
	 * @param length the length of the music file in seconds
	 */
	void updateMeta(int id, int length);

	/// puts a new work onto the queue of the music library
	void addWork(const std::shared_ptr<BaseWork>& work);

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
	sqlite3_stmt* m_getnewfiles;
	sqlite3_stmt* m_updatemeta;

	// mutex for the music database
	thread::Mutex m_mutex;
};

}

#endif
