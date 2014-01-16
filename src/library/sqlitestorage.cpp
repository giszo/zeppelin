#include "sqlitestorage.h"

#include <thread/blocklock.h>

using library::SqliteStorage;

// =====================================================================================================================
SqliteStorage::SqliteStorage(const config::Library& config)
    : Storage(config),
      m_db(NULL)
{
}

// =====================================================================================================================
SqliteStorage::~SqliteStorage()
{
    if (m_db)
	sqlite3_close(m_db);
}

// =====================================================================================================================
void SqliteStorage::open()
{
    if (sqlite3_open(m_config.m_database.empty() ? "library.db" : m_config.m_database.c_str(), &m_db) != SQLITE_OK)
	throw StorageException("unable to open database");

    // turn synchronous mode off and store journal data in memory to speed-up SQLite
    execute("PRAGMA synchronous = OFF");
    execute("PRAGMA journal_mode = MEMORY");
    execute("PRAGMA foreign_keys = ON");

    // artists
    execute(
	R"(CREATE TABLE IF NOT EXISTS artists(
	      id INTEGER PRIMARY KEY,
	      name TEXT,
              UNIQUE(name)))");

    // albums
    execute(
	R"(CREATE TABLE IF NOT EXISTS albums(
	    id INTEGER PRIMARY KEY,
	    artist_id INTEGER,
	    name TEXT,
	    UNIQUE(artist_id, name),
	    FOREIGN KEY(artist_id) REFERENCES artists(id)))");
    execute("CREATE INDEX IF NOT EXISTS albums_artist_id ON albums(artist_id)");

    // directories
    execute(
	R"(CREATE TABLE IF NOT EXISTS directories(
            id INTEGER PRIMARY KEY,
            parent_id INTEGER DEFAULT NULL,
            name TEXT,
            mark INTEGER DEFAULT 1,
            UNIQUE(parent_id, name),
            FOREIGN KEY(parent_id) REFERENCES directories(id)))");

    // files
    execute(
	R"(CREATE TABLE IF NOT EXISTS files(
	    id INTEGER PRIMARY KEY,
	    artist_id INTEGER DEFAULT NULL,
	    album_id INTEGER DEFAULT NULL,
	    directory_id INTEGER,
	    path TEXT,
	    name TEXT,
	    size INTEGER,
	    length INTEGER DEFAULT NULL,
	    title TEXT DEFAULT NULL,
	    year INTEGER DEFAULT NULL,
	    track_index INTEGER DEFAULT NULL,
	    type INTEGER DEFAULT NULL,
	    sampling_rate INTEGER DEFAULT NULL,
	    mark INTEGER DEFAULT 1,
	    UNIQUE(path, name),
	    FOREIGN KEY(artist_id) REFERENCES artists(id),
	    FOREIGN KEY(album_id) REFERENCES albums(id),
	    FOREIGN KEY(directory_id) REFERENCES directories(id)))");
    execute("CREATE INDEX IF NOT EXISTS files_artist_id ON files(artist_id)");

    // prepare statements
    prepareStatement(&m_getDirectory, "SELECT id FROM directories WHERE parent_id IS ? and NAME = ?");
    prepareStatement(&m_getDirectoryById, "SELECT name FROM directories WHERE id = ?");
    prepareStatement(&m_addDirectory, "INSERT INTO directories(parent_id, name) VALUES(?, ?)");
    prepareStatement(&m_getSubdirectories, "SELECT id, name FROM directories WHERE parent_id IS ?");

    prepareStatement(&m_newFile, "INSERT OR IGNORE INTO files(path, name, size, directory_id) VALUES(?, ?, ?, ?)");
    prepareStatement(&m_getFile,
                     R"(SELECT files.path, files.name, files.size, files.length, files.title, files.year,
                               files.track_index, files.type, files.sampling_rate,
                               albums.name,
                               artists.name
                        FROM files LEFT JOIN albums  ON albums.id = files.album_id
                                   LEFT JOIN artists ON artists.id = files.artist_id
                        WHERE files.id = ?)");
    prepareStatement(&m_getFileByPath, "SELECT id FROM files WHERE path = ? AND name = ?");
    prepareStatement(&m_getFilesWithoutMeta, "SELECT id, path, name, size, directory_id FROM files WHERE length IS NULL");
    prepareStatement(&m_getFilesOfArtist,
                     R"(SELECT id, path, name, size, length, artist_id, album_id, title, year, track_index, type, sampling_rate
                        FROM files
                        WHERE artist_id IS ?)");
    // 'name' is used in ORDER BY to try to keep the order of tracks inside an album according to file naming because
    // it may contain information about the index of the track
    prepareStatement(&m_getFilesOfAlbum,
                     R"(SELECT id, path, name, size, length, artist_id, album_id, title, year, track_index, type, sampling_rate
                        FROM files
                        WHERE album_id = ?
                        ORDER BY track_index, name)");
    prepareStatement(&m_getFilesOfDirectory,
                     R"(SELECT id, path, name, size, length, artist_id, album_id, title, year, track_index, type, sampling_rate
                        FROM files
                        WHERE directory_id = ?
                        ORDER BY name)");
    prepareStatement(&m_setFileMark, "UPDATE files SET mark = 1 WHERE id = ?");
    prepareStatement(&m_setDirectoryMark, "UPDATE directories SET mark = 1 WHERE id = ?");

    prepareStatement(&m_setFileMeta,
                     R"(UPDATE files
                        SET artist_id = ?, album_id = ?, length = ?, title = ?, year = ?, track_index = ?, type = ?, sampling_rate = ?
                        WHERE id = ?)");
    prepareStatement(&m_updateFileMeta,
                     R"(UPDATE files
                        SET artist_id = ?, album_id = ?, title = ?, year = ?, track_index = ?
                        WHERE id = ?)");

    // artists
    prepareStatement(&m_addArtist, "INSERT OR IGNORE INTO artists(name) VALUES(?)");
    prepareStatement(&m_getArtists,
                     R"(SELECT artists.id, artists.name, COUNT(DISTINCT files.album_id)
                        FROM files LEFT JOIN artists ON artists.id = files.artist_id
                        GROUP BY files.artist_id)");

    prepareStatement(&m_getArtistIdByName, "SELECT id FROM artists WHERE name = ?");

    // albums
    prepareStatement(&m_addAlbum, "INSERT OR IGNORE INTO albums(artist_id, name) VALUES(?, ?)");
    prepareStatement(&m_getAlbum, "SELECT artist_id, name FROM albums WHERE id = ?");
    prepareStatement(&m_getAlbumIdByName, "SELECT id FROM albums WHERE artist_id IS ? AND name = ?");
    prepareStatement(&m_getAlbums,
                     R"(SELECT albums.id, albums.name, files.artist_id, COUNT(files.id), SUM(files.length)
                        FROM files LEFT JOIN albums ON albums.id = files.album_id
                        GROUP BY files.album_id
                        ORDER BY albums.name)");
    prepareStatement(&m_getAlbumsByArtist,
                     R"(SELECT albums.id, albums.name, COUNT(files.id), SUM(files.length)
                        FROM files LEFT JOIN albums ON albums.id = files.album_id
                        WHERE files.artist_id = ?
                        GROUP BY files.album_id
                        ORDER BY albums.name)");

    // mark
    prepareStatement(&m_clearFileMarks, "UPDATE files SET mark = 0");
    prepareStatement(&m_clearDirectoryMarks, "UPDATE directories SET mark = 0");

    prepareStatement(&m_deleteNonMarkedFiles, "DELETE FROM files WHERE mark = 0");
    prepareStatement(&m_deleteNonMarkedDirectories, "DELETE FROM directories WHERE mark = 0");
}

// =====================================================================================================================
std::shared_ptr<library::Directory> SqliteStorage::getDirectory(int id)
{
    std::shared_ptr<Directory> directory;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getDirectoryById, 1, id);

    if (sqlite3_step(m_getDirectoryById) != SQLITE_ROW)
	throw FileNotFoundException("directory not found with ID");

    directory = std::make_shared<Directory>(
	id,
	getText(m_getDirectoryById, 0) // name
    );

    sqlite3_reset(m_getDirectoryById);

    return directory;
}

// =====================================================================================================================
int SqliteStorage::ensureDirectory(const std::string& name, int parentId)
{
    thread::BlockLock bl(m_mutex);

    // first try to get the directory from the database
    if (parentId == -1)
	sqlite3_bind_null(m_getDirectory, 1);
    else
	sqlite3_bind_int(m_getDirectory, 1, parentId);
    bindText(m_getDirectory, 2, name);
    if (sqlite3_step(m_getDirectory) == SQLITE_ROW)
    {
	int id = sqlite3_column_int(m_getDirectory, 0);
	sqlite3_reset(m_getDirectory);

	// set mark on the directory
	sqlite3_bind_int(m_setDirectoryMark, 1, id);
	sqlite3_step(m_setDirectoryMark);
	sqlite3_reset(m_setDirectoryMark);

	return id;
    }
    sqlite3_reset(m_getDirectory);

    // directory not found - insert it now ...
    if (parentId == -1)
	sqlite3_bind_null(m_addDirectory, 1);
    else
	sqlite3_bind_int(m_addDirectory, 1, parentId);
    bindText(m_addDirectory, 2, name);
    sqlite3_step(m_addDirectory);
    sqlite3_reset(m_addDirectory);

    return sqlite3_last_insert_rowid(m_db);
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::Directory>> SqliteStorage::listSubdirectories(int id)
{
    std::vector<std::shared_ptr<library::Directory>> directories;

    if (id == -1)
	sqlite3_bind_null(m_getSubdirectories, 1);
    else
	sqlite3_bind_int(m_getSubdirectories, 1, id);

    while (sqlite3_step(m_getSubdirectories) == SQLITE_ROW)
    {
	directories.push_back(std::make_shared<Directory>(
	    sqlite3_column_int(m_getSubdirectories, 0),
	    getText(m_getSubdirectories, 1)
	));
    }

    sqlite3_reset(m_getSubdirectories);

    return directories;
}

// =====================================================================================================================
bool SqliteStorage::addFile(File& file)
{
    thread::BlockLock bl(m_mutex);

    // first check whether the file already exists
    int id = getFileIdByPath(file.m_path, file.m_name);

    // the file was found ...
    if (id != -1)
    {
	// set mark on the file
	sqlite3_bind_int(m_setFileMark, 1, id);
	sqlite3_step(m_setFileMark);
	sqlite3_reset(m_setFileMark);

	return false;
    }

    // add the new file
    bindText(m_newFile, 1, file.m_path);
    bindText(m_newFile, 2, file.m_name);
    sqlite3_bind_int64(m_newFile, 3, file.m_size);
    sqlite3_bind_int(m_newFile, 4, file.m_directoryId);
    sqlite3_step(m_newFile);
    sqlite3_reset(m_newFile);

    // set the ID of the new file
    file.m_id = sqlite3_last_insert_rowid(m_db);

    return true;
}

// =====================================================================================================================
void SqliteStorage::clearMark()
{
    thread::BlockLock bl(m_mutex);

    sqlite3_step(m_clearFileMarks);
    sqlite3_reset(m_clearFileMarks);
    sqlite3_step(m_clearDirectoryMarks);
    sqlite3_reset(m_clearDirectoryMarks);
}

// =====================================================================================================================
void SqliteStorage::deleteNonMarked()
{
    thread::BlockLock bl(m_mutex);

    sqlite3_step(m_deleteNonMarkedFiles);
    sqlite3_reset(m_deleteNonMarkedFiles);
    sqlite3_step(m_deleteNonMarkedDirectories);
    sqlite3_reset(m_deleteNonMarkedDirectories);
}

// =====================================================================================================================
std::shared_ptr<library::File> SqliteStorage::getFile(int id)
{
    std::shared_ptr<File> file;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getFile, 1, id);

    if (sqlite3_step(m_getFile) != SQLITE_ROW)
	throw FileNotFoundException("file not found with ID");

    file = std::make_shared<File>(
	id,
	getText(m_getFile, 0), // path
	getText(m_getFile, 1), // name
	sqlite3_column_int64(m_getFile, 2), // size
	sqlite3_column_int(m_getFile, 3), // length
	getText(m_getFile, 10), // artist
	getText(m_getFile, 9), // album
	getText(m_getFile, 4), // title
	sqlite3_column_int(m_getFile, 5), // year
	sqlite3_column_int(m_getFile, 6), // track index
	static_cast<codec::Type>(sqlite3_column_int(m_getFile, 7)), // type
	sqlite3_column_int(m_getFile, 8) // sampling rate
    );

    sqlite3_reset(m_getFile);

    return file;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFilesWithoutMetadata()
{
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    while (sqlite3_step(m_getFilesWithoutMeta) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getFilesWithoutMeta, 0),
	    sqlite3_column_int(m_getFilesWithoutMeta, 4),
	    getText(m_getFilesWithoutMeta, 1),
	    getText(m_getFilesWithoutMeta, 2),
	    sqlite3_column_int64(m_getFilesWithoutMeta, 3)
	));
    }
    sqlite3_reset(m_getFilesWithoutMeta);

    return files;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFilesOfArtist(int artistId)
{
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    if (artistId == -1)
	sqlite3_bind_null(m_getFilesOfArtist, 1);
    else
	sqlite3_bind_int(m_getFilesOfArtist, 1, artistId);

    while (sqlite3_step(m_getFilesOfArtist) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getFilesOfArtist, 0), // id
	    getText(m_getFilesOfArtist, 1), // path
	    getText(m_getFilesOfArtist, 2), // name
	    sqlite3_column_int64(m_getFilesOfArtist, 3), // size
	    sqlite3_column_int(m_getFilesOfArtist, 4), // length
		sqlite3_column_int(m_getFilesOfArtist, 5), // artist
		sqlite3_column_int(m_getFilesOfArtist, 6), // album
	    getText(m_getFilesOfArtist, 7), // title
	    sqlite3_column_int(m_getFilesOfArtist, 8), // year
	    sqlite3_column_int(m_getFilesOfArtist, 9), // track index
	    static_cast<codec::Type>(sqlite3_column_int(m_getFilesOfArtist, 10)), // type
	    sqlite3_column_int(m_getFilesOfArtist, 11) // sampling rate
	));
    }
    sqlite3_reset(m_getFilesOfArtist);

    return files;

}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFilesOfAlbum(int albumId)
{
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getFilesOfAlbum, 1, albumId);

    while (sqlite3_step(m_getFilesOfAlbum) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getFilesOfAlbum, 0), // id
	    getText(m_getFilesOfAlbum, 1), // path
	    getText(m_getFilesOfAlbum, 2), // name
	    sqlite3_column_int64(m_getFilesOfAlbum, 3), // size
	    sqlite3_column_int(m_getFilesOfAlbum, 4), // length
	    sqlite3_column_int(m_getFilesOfAlbum, 5), // artist
	    sqlite3_column_int(m_getFilesOfAlbum, 6), // album
	    getText(m_getFilesOfAlbum, 7), // title
	    sqlite3_column_int(m_getFilesOfAlbum, 8), // year
	    sqlite3_column_int(m_getFilesOfAlbum, 9), // track index
	    static_cast<codec::Type>(sqlite3_column_int(m_getFilesOfAlbum, 10)), // type
	    sqlite3_column_int(m_getFilesOfAlbum, 11) // sampling rate
	));
    }
    sqlite3_reset(m_getFilesOfAlbum);

    return files;

}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFilesOfDirectory(int directoryId)
{
    std::vector<std::shared_ptr<File>> files;

    // root directory does not contain any files
    if (directoryId == -1)
	return files;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getFilesOfDirectory, 1, directoryId);

    while (sqlite3_step(m_getFilesOfDirectory) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getFilesOfDirectory, 0), // id
	    getText(m_getFilesOfDirectory, 1), // path
	    getText(m_getFilesOfDirectory, 2), // name
	    sqlite3_column_int64(m_getFilesOfDirectory, 3), // size
	    sqlite3_column_int(m_getFilesOfDirectory, 4), // length
	    sqlite3_column_int(m_getFilesOfDirectory, 5), // artist
	    sqlite3_column_int(m_getFilesOfDirectory, 6), // album
	    getText(m_getFilesOfDirectory, 7), // title
	    sqlite3_column_int(m_getFilesOfDirectory, 8), // year
	    sqlite3_column_int(m_getFilesOfDirectory, 9), // track index
	    static_cast<codec::Type>(sqlite3_column_int(m_getFilesOfDirectory, 10)), // type
	    sqlite3_column_int(m_getFilesOfDirectory, 11) // sampling rate
	));
    }
    sqlite3_reset(m_getFilesOfDirectory);

    return files;

}

// =====================================================================================================================
void SqliteStorage::setFileMetadata(const library::File& file)
{
    thread::BlockLock bl(m_mutex);

    int artistId = getArtistId(file);
    int albumId = getAlbumId(file, artistId);

    if (artistId == -1)
	sqlite3_bind_null(m_setFileMeta, 1);
    else
	sqlite3_bind_int(m_setFileMeta, 1, artistId);
    if (albumId == -1)
	sqlite3_bind_null(m_setFileMeta, 2);
    else
	sqlite3_bind_int(m_setFileMeta, 2, albumId);
    sqlite3_bind_int(m_setFileMeta, 3, file.m_length);
    bindText(m_setFileMeta, 4, file.m_title);
    sqlite3_bind_int(m_setFileMeta, 5, file.m_year);
    sqlite3_bind_int(m_setFileMeta, 6, file.m_trackIndex);
    sqlite3_bind_int(m_setFileMeta, 7, file.m_type);
    sqlite3_bind_int(m_setFileMeta, 8, file.m_samplingRate);
    sqlite3_bind_int(m_setFileMeta, 9, file.m_id);

    sqlite3_step(m_setFileMeta);
    sqlite3_reset(m_setFileMeta);
}

// =====================================================================================================================
void SqliteStorage::updateFileMetadata(const File& file)
{
    int artistId = getArtistId(file);
    int albumId = getAlbumId(file, artistId);

    if (artistId == -1)
	sqlite3_bind_null(m_updateFileMeta, 1);
    else
	sqlite3_bind_int(m_updateFileMeta, 1, artistId);
    if (albumId == -1)
	sqlite3_bind_null(m_updateFileMeta, 2);
    else
	sqlite3_bind_int(m_updateFileMeta, 2, albumId);
    bindText(m_updateFileMeta, 3, file.m_title);
    sqlite3_bind_int(m_updateFileMeta, 4, file.m_year);
    sqlite3_bind_int(m_updateFileMeta, 5, file.m_trackIndex);
    sqlite3_bind_int(m_updateFileMeta, 6, file.m_id);
    sqlite3_step(m_updateFileMeta);
    sqlite3_reset(m_updateFileMeta);
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::Artist>> SqliteStorage::getArtists()
{
    std::vector<std::shared_ptr<Artist>> artists;

    thread::BlockLock bl(m_mutex);

    while (sqlite3_step(m_getArtists) == SQLITE_ROW)
    {
	std::shared_ptr<Artist> artist = std::make_shared<Artist>(
	    sqlite3_column_type(m_getArtists, 0) == SQLITE_NULL ? -1 : sqlite3_column_int(m_getArtists, 0),
	    getText(m_getArtists, 1),
	    sqlite3_column_int(m_getArtists, 2));
	artists.push_back(artist);
    }

    sqlite3_reset(m_getArtists);

    return artists;
}

// =====================================================================================================================
std::shared_ptr<library::Album> SqliteStorage::getAlbum(int id)
{
    std::shared_ptr<Album> album;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getAlbum, 1, id);

    if (sqlite3_step(m_getAlbum) != SQLITE_ROW)
	throw FileNotFoundException("album not found"); // TODO: use a new exception here!

    album = std::make_shared<Album>(
	id,
	getText(m_getAlbum, 1),
	sqlite3_column_int(m_getAlbum, 0),
	0,
	0);

    sqlite3_reset(m_getAlbum);

    return album;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::Album>> SqliteStorage::getAlbums()
{
    std::vector<std::shared_ptr<Album>> albums;

    thread::BlockLock bl(m_mutex);

    while (sqlite3_step(m_getAlbums) == SQLITE_ROW)
    {
	std::shared_ptr<Album> album = std::make_shared<Album>(
	    sqlite3_column_int(m_getAlbums, 0),
	    getText(m_getAlbums, 1),
	    sqlite3_column_int(m_getAlbums, 2),
	    sqlite3_column_int(m_getAlbums, 3),
	    sqlite3_column_int(m_getAlbums, 4));
	albums.push_back(album);
    }

    sqlite3_reset(m_getAlbums);

    return albums;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::Album>> SqliteStorage::getAlbumsByArtist(int artistId)
{
    std::vector<std::shared_ptr<Album>> albums;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getAlbumsByArtist, 1, artistId);

    while (sqlite3_step(m_getAlbumsByArtist) == SQLITE_ROW)
    {
	std::shared_ptr<Album> album = std::make_shared<Album>(
	    sqlite3_column_int(m_getAlbumsByArtist, 0),
	    getText(m_getAlbumsByArtist, 1),
	    artistId,
	    sqlite3_column_int(m_getAlbumsByArtist, 2),
	    sqlite3_column_int(m_getAlbumsByArtist, 3));
	albums.push_back(album);
    }

    sqlite3_reset(m_getAlbumsByArtist);

    return albums;
}

// =====================================================================================================================
void SqliteStorage::execute(const std::string& sql)
{
    char* error;

    if (sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &error) != SQLITE_OK)
	throw StorageException("unable to execute query");
}

// =====================================================================================================================
void SqliteStorage::prepareStatement(sqlite3_stmt** stmt, const std::string& sql)
{
    if (sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, stmt, NULL) != SQLITE_OK)
	throw StorageException("unable to prepare statement: " + sql);
}

// =====================================================================================================================
std::string SqliteStorage::getText(sqlite3_stmt* stmt, int col)
{
    const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));

    if (!s)
	return "";

    return s;
}

// =====================================================================================================================
void SqliteStorage::bindText(sqlite3_stmt* stmt, int col, const std::string& s)
{
    sqlite3_bind_text(stmt, col, s.c_str(), s.length(), NULL);
}

// =====================================================================================================================
int SqliteStorage::getFileIdByPath(const std::string& path, const std::string& name)
{
    int id;

    bindText(m_getFileByPath, 1, path);
    bindText(m_getFileByPath, 2, name);

    if (sqlite3_step(m_getFileByPath) == SQLITE_ROW)
	id = sqlite3_column_int(m_getFileByPath, 0);
    else
	id = -1;

    sqlite3_reset(m_getFileByPath);

    return id;
}

// =====================================================================================================================
int SqliteStorage::getArtistId(const File& file)
{
    int id;

    if (file.m_artist.empty())
	return -1;

    bindText(m_addArtist, 1, file.m_artist);
    if (sqlite3_step(m_addArtist) != SQLITE_DONE)
	throw StorageException("unable to insert artist");
    sqlite3_reset(m_addArtist);

    bindText(m_getArtistIdByName, 1, file.m_artist);
    if (sqlite3_step(m_getArtistIdByName) != SQLITE_ROW)
	throw StorageException("unable to get artist after inserting!");
    id = sqlite3_column_int(m_getArtistIdByName, 0);
    sqlite3_reset(m_getArtistIdByName);

    return id;
}

// =====================================================================================================================
int SqliteStorage::getAlbumId(const File& file, int artistId)
{
    int id;

    if (file.m_album.empty())
	return -1;

    if (artistId == -1)
	sqlite3_bind_null(m_addAlbum, 1);
    else
	sqlite3_bind_int(m_addAlbum, 1, artistId);
    bindText(m_addAlbum, 2, file.m_album);
    if (sqlite3_step(m_addAlbum) != SQLITE_DONE)
	throw StorageException("unable to insert album");
    sqlite3_reset(m_addAlbum);

    if (artistId == -1)
	sqlite3_bind_null(m_getAlbumIdByName, 1);
    else
	sqlite3_bind_int(m_getAlbumIdByName, 1, artistId);
    bindText(m_getAlbumIdByName, 2, file.m_album);
    if (sqlite3_step(m_getAlbumIdByName) != SQLITE_ROW)
	throw StorageException("unable to get album after inserting!");
    id = sqlite3_column_int(m_getAlbumIdByName, 0);
    sqlite3_reset(m_getAlbumIdByName);

    return id;
}
