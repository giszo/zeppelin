#include "sqlitestorage.h"

#include <thread/blocklock.h>

using library::SqliteStorage;

// =====================================================================================================================
SqliteStorage::SqliteStorage()
    : m_db(NULL)
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
    std::string sql;

    if (sqlite3_open("library.db", &m_db) != SQLITE_OK)
	throw StorageException("unable to open database");

    // create db tables
    execute(
	R"(CREATE TABLE IF NOT EXISTS artists(
	      id INTEGER PRIMARY KEY,
	      name TEXT,
              UNIQUE(name)))");
    execute(
	R"(CREATE TABLE IF NOT EXISTS albums(
	    id INTEGER PRIMARY KEY,
	    artist_id INTEGER,
	    name TEXT,
	    UNIQUE(artist_id, name),
	    FOREIGN KEY(artist_id) REFERENCES artists(id)))");
    execute(
	R"(CREATE TABLE IF NOT EXISTS files(
	    id INTEGER PRIMARY KEY,
	    artist_id INTEGER DEFAULT NULL,
	    album_id INTEGER DEFAULT NULL,
	    path TEXT,
	    name TEXT,
	    length INTEGER DEFAULT NULL,
	    title TEXT DEFAULT NULL,
	    year INTEGER DEFAULT NULL,
	    UNIQUE(path, name),
	    FOREIGN KEY(artist_id) REFERENCES artists(id),
	    FOREIGN KEY(album_id) REFERENCES albums(id)))");

    // prepare statements
    prepareStatement(&m_newFile, "INSERT OR IGNORE INTO files(path, name) VALUES(?, ?)");
    prepareStatement(&m_getFile,
                     R"(SELECT files.path, files.name, files.length, files.title, files.year,
                               albums.name,
                               artists.name
                        FROM files LEFT JOIN albums  ON albums.id = files.album_id
                                   LEFT JOIN artists ON artists.id = files.artist_id
                        WHERE files.id = ?)");
    prepareStatement(&m_getFiles,
                     R"(SELECT files.id, files.path, files.name, files.length, files.title, files.year,
                               albums.name,
                               artists.name
                        FROM files LEFT JOIN albums  ON albums.id = files.album_id
                                   LEFT JOIN artists ON artists.id = files.artist_id)");
    prepareStatement(&m_getFilesWithoutMeta, "SELECT id, path, name FROM files WHERE length IS NULL LIMIT ?");
    prepareStatement(&m_updateFileMeta,
                     R"(UPDATE files
                        SET artist_id = ?, album_id = ?, length = ?, title = ?, year = ?
                        WHERE id = ?)");

    // artists
    prepareStatement(&m_addArtist, "INSERT OR IGNORE INTO artists(name) VALUES(?)");
    prepareStatement(&m_getArtists, "SELECT id, name FROM artists");
    prepareStatement(&m_getArtistByName, "SELECT id FROM artists WHERE name = ?");

    // albums
    prepareStatement(&m_addAlbum, "INSERT OR IGNORE INTO albums(artist_id, name) VALUES(?, ?)");
    prepareStatement(&m_getAlbumByName, "SELECT id FROM albums WHERE artist_id = ? AND name = ?");
    prepareStatement(&m_getAlbums, R"(SELECT albums.id, albums.name, artists.id, artists.name
                                      FROM albums LEFT JOIN artists ON albums.artist_id = artists.id)");
    prepareStatement(&m_getAlbumsByArtist, "SELECT id, name FROM albums WHERE artist_id = ?");
}

// =====================================================================================================================
void SqliteStorage::addFile(File& file)
{
    thread::BlockLock bl(m_mutex);

    bindText(m_newFile, 1, file.m_path);
    bindText(m_newFile, 2, file.m_name);
    sqlite3_step(m_newFile);
    sqlite3_reset(m_newFile);
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
	sqlite3_column_int(m_getFile, 2), // length
	getText(m_getFile, 6), // artist
	getText(m_getFile, 5), // album
	getText(m_getFile, 3), // title
	sqlite3_column_int(m_getFile, 4) // year
    );

    sqlite3_reset(m_getFile);

    return file;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFiles()
{
    int r;
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    while ((r = sqlite3_step(m_getFiles)) == SQLITE_ROW)
    {
	std::shared_ptr<File> file = std::make_shared<File>(
	    sqlite3_column_int(m_getFiles, 0), // id
	    getText(m_getFiles, 1), // path
	    getText(m_getFiles, 2), // name
	    sqlite3_column_int(m_getFiles, 3), // length
	    getText(m_getFiles, 7), // artist
	    getText(m_getFiles, 6), // album
	    getText(m_getFiles, 4), // title
	    sqlite3_column_int(m_getFiles, 5) // year
	);
	files.push_back(file);
    }
    sqlite3_reset(m_getFiles);

    return files;
}

// =====================================================================================================================
std::vector<std::shared_ptr<library::File>> SqliteStorage::getFilesWithoutMetadata(int amount)
{
    int r;
    std::vector<std::shared_ptr<File>> files;

    thread::BlockLock bl(m_mutex);

    sqlite3_bind_int(m_getFilesWithoutMeta, 1, amount);

    while ((r = sqlite3_step(m_getFilesWithoutMeta)) == SQLITE_ROW)
    {
	files.push_back(std::make_shared<File>(
	    sqlite3_column_int(m_getFilesWithoutMeta, 0),
	    getText(m_getFilesWithoutMeta, 1),
	    getText(m_getFilesWithoutMeta, 2)
	));
    }
    sqlite3_reset(m_getFilesWithoutMeta);

    return files;
}

// =====================================================================================================================
void SqliteStorage::updateFileMetadata(const library::File& file)
{
    int artistId;
    int albumId;

    thread::BlockLock bl(m_mutex);

    // handle artist
    if (file.m_artist.empty())
	artistId = -1;
    else
    {
	bindText(m_addArtist, 1, file.m_artist);
	if (sqlite3_step(m_addArtist) != SQLITE_DONE)
	    throw StorageException("unable to insert artist");
	sqlite3_reset(m_addArtist);

	bindText(m_getArtistByName, 1, file.m_artist);
	if (sqlite3_step(m_getArtistByName) != SQLITE_ROW)
	    throw StorageException("unable to get artist after inserting!");
	artistId = sqlite3_column_int(m_getArtistByName, 0);
	sqlite3_reset(m_getArtistByName);
    }

    // handle album
    if (file.m_album.empty())
	albumId = -1;
    else
    {
	if (artistId == -1)
	    sqlite3_bind_null(m_addAlbum, 1);
	else
	    sqlite3_bind_int(m_addAlbum, 1, artistId);
	bindText(m_addAlbum, 2, file.m_album);
	if (sqlite3_step(m_addAlbum) != SQLITE_DONE)
	    throw StorageException("unable to insert album");
	sqlite3_reset(m_addAlbum);

	if (artistId == -1)
	    sqlite3_bind_null(m_getAlbumByName, 1);
	else
	    sqlite3_bind_int(m_getAlbumByName, 1, artistId);
	bindText(m_getAlbumByName, 2, file.m_album);
	if (sqlite3_step(m_getAlbumByName) != SQLITE_ROW)
	    throw StorageException("unable to get album after inserting!");
	albumId = sqlite3_column_int(m_getAlbumByName, 0);
	sqlite3_reset(m_getAlbumByName);
    }

    if (artistId == -1)
	sqlite3_bind_null(m_updateFileMeta, 1);
    else
	sqlite3_bind_int(m_updateFileMeta, 1, artistId);
    if (albumId == -1)
	sqlite3_bind_null(m_updateFileMeta, 2);
    else
	sqlite3_bind_int(m_updateFileMeta, 2, albumId);
    sqlite3_bind_int(m_updateFileMeta, 3, file.m_length);
    bindText(m_updateFileMeta, 4, file.m_title);
    sqlite3_bind_int(m_updateFileMeta, 5, file.m_year);
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
	    sqlite3_column_int(m_getArtists, 0),
	    getText(m_getArtists, 1));
	artists.push_back(artist);
    }

    sqlite3_reset(m_getArtists);

    return artists;
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
	    getText(m_getAlbums, 3));
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
	    getText(m_getAlbumsByArtist, 1));
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
