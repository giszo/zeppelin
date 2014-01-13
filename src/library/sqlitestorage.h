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
	SqliteStorage(const config::Library& config);
	~SqliteStorage();

	void open();

	int ensureDirectory(const std::string& name, int parentId) override;
	std::vector<std::shared_ptr<Directory>> listSubdirectories(int id) override;

	bool addFile(File& file) override;

	void clearMark() override;
	void deleteNonMarked() override;

	std::shared_ptr<File> getFile(int id) override;
	std::vector<std::shared_ptr<File>> getFilesWithoutMetadata() override;
	std::vector<std::shared_ptr<File>> getFilesOfArtist(int artistId) override;
	std::vector<std::shared_ptr<File>> getFilesOfAlbum(int albumId) override;
	std::vector<std::shared_ptr<File>> getFilesOfDirectory(int directoryId) override;

	void setFileMetadata(const File& file) override;
	void updateFileMetadata(const File& file) override;

	std::vector<std::shared_ptr<Artist>> getArtists() override;

	std::shared_ptr<Album> getAlbum(int id) override;
	std::vector<std::shared_ptr<Album>> getAlbums() override;
	std::vector<std::shared_ptr<Album>> getAlbumsByArtist(int artistId) override;

    private:
	void execute(const std::string& sql);
	void prepareStatement(sqlite3_stmt** stmt, const std::string& sql);

	std::string getText(sqlite3_stmt* stmt, int col);

	void bindText(sqlite3_stmt* stmt, int col, const std::string& s);

	int getFileIdByPath(const std::string& path, const std::string& name);
	int getArtistId(const File& file);
	int getAlbumId(const File& file, int artistId);

    private:
	// the database to store the music library
	sqlite3* m_db;

	sqlite3_stmt* m_getDirectory;
	sqlite3_stmt* m_addDirectory;
	sqlite3_stmt* m_getSubdirectories;

	sqlite3_stmt* m_newFile;

	sqlite3_stmt* m_getFile;
	sqlite3_stmt* m_getFileByPath;
	sqlite3_stmt* m_getFilesWithoutMeta;
	sqlite3_stmt* m_getFilesOfArtist;
	sqlite3_stmt* m_getFilesOfAlbum;
	sqlite3_stmt* m_getFilesOfDirectory;
	sqlite3_stmt* m_setFileMark;

	sqlite3_stmt* m_setFileMeta;
	sqlite3_stmt* m_updateFileMeta;

	/// artist handling
	sqlite3_stmt* m_addArtist;
	sqlite3_stmt* m_getArtists;
	sqlite3_stmt* m_getArtistIdByName;

	/// album handling
	sqlite3_stmt* m_addAlbum;
	sqlite3_stmt* m_getAlbum;
	sqlite3_stmt* m_getAlbumIdByName;
	sqlite3_stmt* m_getAlbums;
	sqlite3_stmt* m_getAlbumsByArtist;

	/// mark handling
	sqlite3_stmt* m_clearMark;
	sqlite3_stmt* m_deleteNonMarked;

	// mutex for the music database
	thread::Mutex m_mutex;
};

}

#endif
