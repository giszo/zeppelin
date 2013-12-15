#ifndef LIBRARY_SQLITESTORAGE_H_INCLUDED
#define LIBRARY_SQLITESTORAGE_H_INCLUDED

#include "storage.h"

#include <thread/mutex.h>

#include <sqlite3.h>

namespace library
{

/**
 * Storage backend for the music library based on SQLite3.
 */
class SqliteStorage : public Storage
{
    public:
	SqliteStorage();
	~SqliteStorage();

	void open();

	bool addFile(File& file) override;

	std::shared_ptr<File> getFile(int id) override;
	std::vector<std::shared_ptr<library::File>> getFiles() override;
	std::vector<std::shared_ptr<library::File>> getFilesWithoutMetadata() override;
	std::vector<std::shared_ptr<File>> getFilesOfAlbum(int albumId) override;

	void updateFileMetadata(const library::File& file) override;

	std::vector<std::shared_ptr<Artist>> getArtists() override;

	std::vector<std::shared_ptr<Album>> getAlbums() override;
	std::vector<std::shared_ptr<Album>> getAlbumsByArtist(int artistId) override;

    private:
	void execute(const std::string& sql);
	void prepareStatement(sqlite3_stmt** stmt, const std::string& sql);

	std::string getText(sqlite3_stmt* stmt, int col);

	void bindText(sqlite3_stmt* stmt, int col, const std::string& s);

    private:
	// the database to store the music library
	sqlite3* m_db;

	sqlite3_stmt* m_newFile;

	sqlite3_stmt* m_getFile;
	sqlite3_stmt* m_getFileByPath;
	sqlite3_stmt* m_getFiles;
	sqlite3_stmt* m_getFilesWithoutMeta;
	sqlite3_stmt* m_getFilesOfAlbum;

	sqlite3_stmt* m_updateFileMeta;

	/// artist handling
	sqlite3_stmt* m_addArtist;
	sqlite3_stmt* m_getArtists;
	sqlite3_stmt* m_getArtistIdByName;

	/// album handling
	sqlite3_stmt* m_addAlbum;
	sqlite3_stmt* m_getAlbumIdByName;
	sqlite3_stmt* m_getAlbums;
	sqlite3_stmt* m_getAlbumsByArtist;

	// mutex for the music database
	thread::Mutex m_mutex;
};

}

#endif
