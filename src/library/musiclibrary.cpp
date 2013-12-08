#include "musiclibrary.h"

#include <thread/blocklock.h>

#include <iostream>

using library::MusicLibrary;
using library::ScanDirectory;

// =====================================================================================================================
MusicLibrary::MusicLibrary()
    : m_db(NULL)
{
    // start the thread of the worker
    m_worker.start();
}

// =====================================================================================================================
MusicLibrary::~MusicLibrary()
{
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
		 "CREATE TABLE IF NOT EXISTS files(id INTEGER PRIMARY KEY, path TEXT, name TEXT, length INTEGER DEFAULT NULL, UNIQUE(path, name))",
		 NULL, NULL,
		 &error);

    // prepare statements
    sql = "INSERT INTO files(path, name) VALUES(?, ?)";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_newfile, NULL);
    sql = "SELECT path, name, length FROM files WHERE id = ?";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_getfile, NULL);
    sql = "SELECT id, path, name, length FROM files";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_listfiles, NULL);
    sql = "SELECT id, path, name FROM files WHERE length IS NULL LIMIT ?";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_getnewfiles, NULL);
    sql = "UPDATE files SET length = ? WHERE id = ?";
    sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, &m_updatemeta, NULL);
}

// =====================================================================================================================
std::shared_ptr<library::File> MusicLibrary::getFile(int id)
{
    std::shared_ptr<File> file;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getfile, 1, id);

    if (sqlite3_step(m_getfile) != SQLITE_ROW)
	throw FileNotFoundException("file not found with ID");

    file = std::make_shared<File>(
	id,
	reinterpret_cast<const char*>(sqlite3_column_text(m_getfile, 0)),
	reinterpret_cast<const char*>(sqlite3_column_text(m_getfile, 1)),
	sqlite3_column_int(m_getfile, 2)
    );

    sqlite3_reset(m_getfile);

    return file;
}

// =====================================================================================================================
std::vector<library::File> MusicLibrary::getFileList()
{
    int r;
    std::vector<File> files;

    thread::BlockLock bl(m_mutex);

    while ((r = sqlite3_step(m_listfiles)) == SQLITE_ROW)
    {
	File f{
	    sqlite3_column_int(m_listfiles, 0),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_listfiles, 1)),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_listfiles, 2)),
	    sqlite3_column_int(m_listfiles, 3)
	};
	files.push_back(f);
    }
    sqlite3_reset(m_listfiles);

    return files;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> MusicLibrary::getNewFiles(int amount)
{
    int r;
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getnewfiles, 1, amount);

    while ((r = sqlite3_step(m_getnewfiles)) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getnewfiles, 0),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_getnewfiles, 1)),
	    reinterpret_cast<const char*>(sqlite3_column_text(m_getnewfiles, 2)),
	    0
	));
    }
    sqlite3_reset(m_listfiles);

    return files;
}

// =====================================================================================================================
void MusicLibrary::scanDirectory(const std::string& path)
{
    addWork(std::make_shared<ScanDirectory>(path, *this));
}

// =====================================================================================================================
void MusicLibrary::updateMeta(int id, int length)
{
    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_updatemeta, 1, length);
    sqlite3_bind_int(m_updatemeta, 2, id);

    sqlite3_step(m_updatemeta);
    sqlite3_reset(m_updatemeta);
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
    thread::BlockLock bl(m_mutex);

    sqlite3_bind_text(m_newfile, 1, path.c_str(), path.length(), NULL);
    sqlite3_bind_text(m_newfile, 2, name.c_str(), name.length(), NULL);

    sqlite3_step(m_newfile);
    sqlite3_reset(m_newfile);
}
