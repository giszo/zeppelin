#ifndef LIBRARY_SQLITESTORAGE_H_INCLUDED
#define LIBRARY_SQLITESTORAGE_H_INCLUDED

#include <zeppelin/library/storage.h>

#include <thread/mutex.h>

#include <sqlite3.h>

namespace config
{
struct Library;
}

namespace library
{

/**
 * Storage backend for the music library based on SQLite3.
 */
class SqliteStorage : public zeppelin::library::Storage
{
    public:
	SqliteStorage();
	~SqliteStorage();

	void open(const config::Library& config);

	zeppelin::library::Statistics getStatistics() override;

	std::shared_ptr<zeppelin::library::Directory> getDirectory(int id) override;
	int ensureDirectory(const std::string& name, int parentId) override;
	std::vector<std::shared_ptr<zeppelin::library::Directory>> listSubdirectories(int id) override;

	bool addFile(zeppelin::library::File& file) override;

	void clearMark() override;
	void deleteNonMarked() override;

	std::vector<std::shared_ptr<zeppelin::library::File>> getFilesWithoutMetadata() override;

	std::vector<std::shared_ptr<zeppelin::library::File>> getFiles(const std::vector<int>& ids) override;
	std::vector<int> getFileIdsOfAlbum(int albumId) override;
	std::vector<int> getFileIdsOfDirectory(int directoryId) override;

	void setFileMetadata(const zeppelin::library::File& file) override;
	void updateFileMetadata(const zeppelin::library::File& file) override;

	std::vector<std::shared_ptr<zeppelin::library::Artist>> getArtists(const std::vector<int>& ids) override;

	std::vector<int> getAlbumIdsByArtist(int artistId) override;
	std::vector<std::shared_ptr<zeppelin::library::Album>> getAlbums(const std::vector<int>& ids) override;

    private:
	void execute(const std::string& sql);
	void prepareStatement(sqlite3_stmt** stmt, const std::string& sql);

	int getFileIdByPath(const std::string& path, const std::string& name);
	int getArtistId(const zeppelin::library::File& file);
	int getAlbumId(const zeppelin::library::File& file, int artistId);

	static void serializeIntList(std::ostringstream& stream, const std::vector<int>& list);

    private:
	struct StatementHolder
	{
	    StatementHolder(sqlite3* db, const std::string& query);
	    StatementHolder(sqlite3_stmt* stmt);
	    ~StatementHolder();

	    void bindNull(int col);
	    void bindInt(int col, int value);
	    void bindInt64(int col, int64_t value);
	    void bindText(int col, const std::string& value);
	    // a special bind function that binds NULL if the value is -1, otherwise the numeric value
	    void bindIndex(int col, int value);

	    int step();

	    int columnCount();
	    std::string tableName(int col);
	    std::string columnName(int col);
	    int columnType(int col);

	    int getInt(int col);
	    int64_t getInt64(int col);
	    std::string getText(int col);

	    // true when the statement should be finalized by the holder
	    bool m_finalize;
	    sqlite3_stmt* m_stmt;
	};

    private:
	// the database to store the music library
	sqlite3* m_db;

	sqlite3_stmt* m_getDirectory;
	sqlite3_stmt* m_getDirectoryById;
	sqlite3_stmt* m_addDirectory;
	sqlite3_stmt* m_getSubdirectories;

	sqlite3_stmt* m_newFile;

	sqlite3_stmt* m_getFileByPath;
	sqlite3_stmt* m_getFilesWithoutMeta;
	sqlite3_stmt* m_getFileIdsOfAlbum;
	sqlite3_stmt* m_getFileIdsOfDirectory;
	sqlite3_stmt* m_getFileStatistics;

	sqlite3_stmt* m_setFileMark;
	sqlite3_stmt* m_setDirectoryMark;

	sqlite3_stmt* m_setFileMeta;
	sqlite3_stmt* m_updateFileMeta;

	/// artist handling
	sqlite3_stmt* m_addArtist;
	sqlite3_stmt* m_getArtistIdByName;
	sqlite3_stmt* m_getNumOfArtists;

	/// album handling
	sqlite3_stmt* m_addAlbum;
	sqlite3_stmt* m_getAlbumIdByName;
	sqlite3_stmt* m_getAlbumIdsByArtist;
	sqlite3_stmt* m_getNumOfAlbums;

	/// mark handling
	sqlite3_stmt* m_clearFileMarks;
	sqlite3_stmt* m_clearDirectoryMarks;
	sqlite3_stmt* m_deleteNonMarkedFiles;
	sqlite3_stmt* m_deleteNonMarkedDirectories;

	// mutex for the music database
	thread::Mutex m_mutex;
};

}

#endif
