#include "musiclibrary.h"

#include <iostream>

using library::MusicLibrary;
using library::ScanDirectory;

// =====================================================================================================================
MusicLibrary::MusicLibrary()
    : m_db(NULL)
{
    pthread_mutex_init(&m_lock, NULL);

    // start the thread of the worker
    m_worker.start();
}

// =====================================================================================================================
MusicLibrary::~MusicLibrary()
{
    pthread_mutex_destroy(&m_lock);

    if (m_db)
	sqlite3_close(m_db);
}

// =====================================================================================================================
void MusicLibrary::open()
{
    std::string sql;

    // TODO: error handling
    sqlite3_open("library.db", &m_db);

    // create db tables
    char* error;
    sqlite3_exec(m_db,
		 "CREATE TABLE IF NOT EXISTS files(id INTEGER PRIMARY KEY, path TEXT, name TEXT, UNIQUE(path, name))",
		 NULL, NULL,
		 &error);

    // prepare statements
    sql = "INSERT INTO files(path, name) VALUES(?, ?)";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_newfile, NULL);
    sql = "SELECT path, name FROM files WHERE id = ?";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_getfile, NULL);
    sql = "SELECT id, path, name FROM files";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_listfiles, NULL);
}

// =====================================================================================================================
std::shared_ptr<library::File> MusicLibrary::getFile(int id)
{
    std::shared_ptr<File> file;

    pthread_mutex_lock(&m_lock);

    sqlite3_bind_int(m_getfile, 1, id);

    if (sqlite3_step(m_getfile) != SQLITE_ROW)
	throw FileNotFoundException("file not found with ID");

    file = std::make_shared<File>(
	id,
	reinterpret_cast<const char*>(sqlite3_column_text(m_getfile, 0)),
	reinterpret_cast<const char*>(sqlite3_column_text(m_getfile, 1))
    );

    sqlite3_reset(m_getfile);

    pthread_mutex_unlock(&m_lock);

    return file;
}

// =====================================================================================================================
std::vector<library::File> MusicLibrary::getFileList()
{
    int r;
    std::vector<File> files;

    pthread_mutex_lock(&m_lock);

    while ((r = sqlite3_step(m_listfiles)) == SQLITE_ROW)
    {
	File f{
	    sqlite3_column_int(m_listfiles, 0),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_listfiles, 1)),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_listfiles, 2))
	};
	files.push_back(f);
    }
    sqlite3_reset(m_listfiles);

    pthread_mutex_unlock(&m_lock);

    return files;
}

// =====================================================================================================================
void MusicLibrary::scanDirectory(const std::string& path)
{
    m_worker.add(std::make_shared<ScanDirectory>(path, *this));
}

// =====================================================================================================================
void MusicLibrary::directoryFound(const std::string& path)
{
    // recursively scan sub-directories
    m_worker.add(std::make_shared<ScanDirectory>(path, *this));
}

// =====================================================================================================================
void MusicLibrary::musicFound(const std::string& path, const std::string& name)
{
    pthread_mutex_lock(&m_lock);

    sqlite3_bind_text(m_newfile, 1, path.c_str(), path.length(), NULL);
    sqlite3_bind_text(m_newfile, 2, name.c_str(), name.length(), NULL);

    sqlite3_step(m_newfile);
    sqlite3_reset(m_newfile);

    pthread_mutex_unlock(&m_lock);
}
