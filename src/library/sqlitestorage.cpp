/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include "sqlitestorage.h"

#include <config/config.h>
#include <thread/blocklock.h>

#include <zeppelin/logger.h>

#include <sstream>
#include <cstring>

using zeppelin::library::Picture;

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
void SqliteStorage::open(const config::Library& config)
{
    if (sqlite3_open(config.m_database.empty() ? "library.db" : config.m_database.c_str(), &m_db) != SQLITE_OK)
	throw zeppelin::library::StorageException("unable to open database");

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
    execute(
	R"(CREATE TABLE IF NOT EXISTS album_pictures(
	    id INTEGER PRIMARY KEY,
	    album_id INTEGER,
	    type TEXT,
	    mimetype TEXT,
	    data BLOB,
	    UNIQUE(album_id, type),
	    FOREIGN KEY(album_id) REFERENCES albums(id) ON DELETE CASCADE))");

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
	    codec TEXT DEFAULT NULL,
	    channels INTEGER DEFAULT NULL,
	    sample_rate INTEGER DEFAULT NULL,
	    sample_size INTEGER DEFAULT NULL,
	    mark INTEGER DEFAULT 1,
	    UNIQUE(path, name),
	    FOREIGN KEY(artist_id) REFERENCES artists(id),
	    FOREIGN KEY(album_id) REFERENCES albums(id),
	    FOREIGN KEY(directory_id) REFERENCES directories(id)))");
    execute("CREATE INDEX IF NOT EXISTS files_artist_id ON files(artist_id)");
    execute("CREATE INDEX IF NOT EXISTS files_album_id ON files(album_id)");

    // playlists
    execute(
	R"(CREATE TABLE IF NOT EXISTS playlists(
	    id INTEGER PRIMARY KEY,
	    name TEXT))");
    execute(
	R"(CREATE TABLE IF NOT EXISTS playlist_items(
	    id INTEGER PRIMARY KEY,
	    playlist_id INTEGER,
	    type TEXT,
	    item_id INTEGER,
	    FOREIGN KEY(playlist_id) REFERENCES playlists(id) ON DELETE CASCADE))");

    // prepare statements
    prepareStatement(&m_getDirectory, "SELECT id FROM directories WHERE parent_id IS ? and NAME = ?");
    prepareStatement(&m_addDirectory, "INSERT INTO directories(parent_id, name) VALUES(?, ?)");
    prepareStatement(&m_getSubdirectoryIds, "SELECT id FROM directories WHERE parent_id = ?");

    prepareStatement(&m_newFile, "INSERT OR IGNORE INTO files(path, name, size, directory_id) VALUES(?, ?, ?, ?)");
    prepareStatement(&m_getFileByPath, "SELECT id FROM files WHERE path = ? AND name = ?");
    prepareStatement(&m_getFilesWithoutMeta, "SELECT id, directory_id, path, name FROM files WHERE length IS NULL");
    // 'name' is used in ORDER BY to try to keep the order of tracks inside an album according to file naming because
    // it may contain information about the index of the track
    prepareStatement(&m_getFileIdsOfAlbum, "SELECT id FROM files WHERE album_id = ?");
    prepareStatement(&m_getFileIdsOfDirectory, "SELECT id FROM files WHERE directory_id = ?");
    prepareStatement(&m_getFileStatistics, "SELECT COUNT(id), SUM(length), SUM(size) FROM files");

    prepareStatement(&m_setFileMark, "UPDATE files SET mark = 1 WHERE id = ?");
    prepareStatement(&m_setDirectoryMark, "UPDATE directories SET mark = 1 WHERE id = ?");
    prepareStatement(&m_setFileMeta,
                     R"(UPDATE files
                        SET artist_id = ?, album_id = ?, length = ?, title = ?, year = ?, track_index = ?, codec = ?, sample_rate = ?, sample_size = ?, channels = ?
                        WHERE id = ?)");
    prepareStatement(&m_updateFileMeta,
                     R"(UPDATE files
                        SET artist_id = ?, album_id = ?, title = ?, year = ?, track_index = ?
                        WHERE id = ?)");
    prepareStatement(&m_addAlbumPicture, "INSERT OR IGNORE INTO album_pictures(album_id, type, mimetype, data) VALUES(?, ?, ?, ?)");

    // artists
    prepareStatement(&m_addArtist, "INSERT OR IGNORE INTO artists(name) VALUES(?)");
    prepareStatement(&m_getArtistIdByName, "SELECT id FROM artists WHERE name = ?");
    prepareStatement(&m_getNumOfArtists, "SELECT COUNT(id) FROM artists");

    // albums
    prepareStatement(&m_addAlbum, "INSERT OR IGNORE INTO albums(artist_id, name) VALUES(?, ?)");
    prepareStatement(&m_getAlbumIdByName, "SELECT id FROM albums WHERE artist_id IS ? AND name = ?");
    prepareStatement(&m_getAlbumIdsByArtist, "SELECT id FROM albums WHERE artist_id = ?");
    prepareStatement(&m_getNumOfAlbums, "SELECT COUNT(id) FROM albums");


    // playlists
    prepareStatement(&m_createPlaylist, "INSERT INTO playlists(name) VALUES(?)");
    prepareStatement(&m_deletePlaylist, "DELETE FROM playlists WHERE id = ?");
    prepareStatement(&m_addPlaylistItem, "INSERT INTO playlist_items(playlist_id, type, item_id) VALUES(?, ?, ?)");
    prepareStatement(&m_deletePlaylistItem, "DELETE FROM playlist_items WHERE id = ?");

    // mark
    prepareStatement(&m_clearFileMarks, "UPDATE files SET mark = 0");
    prepareStatement(&m_clearDirectoryMarks, "UPDATE directories SET mark = 0");

    prepareStatement(&m_deleteNonMarkedFiles, "DELETE FROM files WHERE mark = 0");
    prepareStatement(&m_deleteNonMarkedDirectories, "DELETE FROM directories WHERE mark = 0");
}

// =====================================================================================================================
zeppelin::library::Statistics SqliteStorage::getStatistics()
{
    zeppelin::library::Statistics stat;

    thread::BlockLock bl(m_mutex);

    {
	StatementHolder stmt(m_getNumOfArtists);
	stmt.step();
	stat.m_numOfArtists = stmt.getInt(0);
    }

    {
	StatementHolder stmt(m_getNumOfAlbums);
	stmt.step();
	stat.m_numOfAlbums = stmt.getInt(0);
    }

    {
	StatementHolder stmt(m_getFileStatistics);
	stmt.step();
	stat.m_numOfFiles = stmt.getInt(0);
	stat.m_sumOfSongLengths = stmt.getInt64(1);
	stat.m_sumOfFileSizes = stmt.getInt64(2);
    }

    return stat;
}

// =====================================================================================================================
std::vector<std::shared_ptr<zeppelin::library::Directory>> SqliteStorage::getDirectories(const std::vector<int>& ids)
{
    std::ostringstream query;
    query << "SELECT id, name, parent_id ";
    query << "FROM directories ";
    if (!ids.empty())
    {
	query << "WHERE id IN (";
	serializeIntList(query, ids);
	query << ") ";
    }

    std::vector<std::shared_ptr<zeppelin::library::Directory>> directories;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	directories.push_back(std::make_shared<zeppelin::library::Directory>(
	    stmt.getInt(0),
	    stmt.getText(1), // name
	    stmt.getInt(2) // parent_id
	));
    }

    return directories;
}

// =====================================================================================================================
std::vector<int> SqliteStorage::getSubdirectoryIdsOfDirectory(int id)
{
    std::vector<int> ids;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_getSubdirectoryIds);
    stmt.bindInt(1, id);

    while (stmt.step() == SQLITE_ROW)
	ids.push_back(stmt.getInt(0));

    return ids;
}

// =====================================================================================================================
int SqliteStorage::ensureDirectory(const std::string& name, int parentId)
{
    thread::BlockLock bl(m_mutex);

    int id;

    // first try to get the directory from the database
    {
	StatementHolder stmt(m_getDirectory);
	stmt.bindIndex(1, parentId);
	stmt.bindText(2, name);

	if (stmt.step() == SQLITE_ROW)
	    id = stmt.getInt(0);
	else
	    id = -1;
    }

    // set mark on the directory and return its id if it was found
    if (id != -1)
    {
	StatementHolder stmt(m_setDirectoryMark);
	stmt.bindInt(1, id);
	stmt.step();

	return id;
    }

    // directory not found - insert it now ...
    {
	StatementHolder stmt(m_addDirectory);
	stmt.bindIndex(1, parentId);
	stmt.bindText(2, name);
	stmt.step();
    }

    return sqlite3_last_insert_rowid(m_db);
}

// =====================================================================================================================
bool SqliteStorage::addFile(zeppelin::library::File& file)
{
    thread::BlockLock bl(m_mutex);

    // first check whether the file already exists
    int id = getFileIdByPath(file.m_path, file.m_name);

    // the file was found ...
    if (id != -1)
    {
	// set mark on the file
	StatementHolder stmt(m_setFileMark);
	stmt.bindInt(1, id);
	stmt.step();

	return false;
    }

    // add the new file
    {
	StatementHolder stmt(m_newFile);
	stmt.bindText(1, file.m_path);
	stmt.bindText(2, file.m_name);
	stmt.bindInt64(3, file.m_size);
	stmt.bindInt(4, file.m_directoryId);
	stmt.step();
    }

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
std::vector<std::shared_ptr<zeppelin::library::File>> SqliteStorage::getFilesWithoutMetadata()
{
    std::vector<std::shared_ptr<zeppelin::library::File>> files;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_getFilesWithoutMeta);

    while (stmt.step() == SQLITE_ROW)
    {
	std::shared_ptr<zeppelin::library::File> file = std::make_shared<zeppelin::library::File>(stmt.getInt(0));
	file->m_directoryId = stmt.getInt(1);
	file->m_path = stmt.getText(2);
	file->m_name = stmt.getText(3);
	files.push_back(file);
    }

    return files;
}

// =====================================================================================================================
std::vector<std::shared_ptr<zeppelin::library::File>> SqliteStorage::getFiles(const std::vector<int>& ids)
{
    std::vector<std::shared_ptr<zeppelin::library::File>> files;

    std::ostringstream query;

    query << "SELECT id, path, name, directory_id, artist_id, album_id, size, length, title, year, track_index, codec, sample_rate, sample_size, channels ";
    query << "FROM files ";
    if (!ids.empty())
    {
	query << "WHERE id IN (";
	serializeIntList(query, ids);
	query << ") ";
    }

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	std::shared_ptr<zeppelin::library::File> file = std::make_shared<zeppelin::library::File>(stmt.getInt(0));
	file->m_path = stmt.getText(1);
	file->m_name = stmt.getText(2);
	file->m_directoryId = stmt.getInt(3);
	file->m_artistId = stmt.getInt(4);
	file->m_albumId = stmt.getInt(5);
	file->m_size = stmt.getInt(6);
	file->m_metadata.reset(new zeppelin::library::Metadata(stmt.getText(11) /* codec */));
	file->m_metadata->setLength(stmt.getInt(7));
	file->m_metadata->setTitle(stmt.getText(8));
	file->m_metadata->setYear(stmt.getInt(9));
	file->m_metadata->setTrackIndex(stmt.getInt(10));
	file->m_metadata->setFormat(stmt.getInt(14), stmt.getInt(12), stmt.getInt(13));
	files.push_back(file);
    }

    return files;
}

// =====================================================================================================================
std::vector<int> SqliteStorage::getFileIdsOfAlbum(int albumId)
{
    std::vector<int> fileIds;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_getFileIdsOfAlbum);
    stmt.bindInt(1, albumId);

    while (stmt.step() == SQLITE_ROW)
	fileIds.push_back(stmt.getInt(0));

    return fileIds;
}

// =====================================================================================================================
std::vector<int> SqliteStorage::getFileIdsOfDirectory(int directoryId)
{
    std::vector<int> fileIds;

    // root directory does not contain any files
    if (directoryId == -1)
	return fileIds;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_getFileIdsOfDirectory);
    stmt.bindInt(1, directoryId);

    while (stmt.step() == SQLITE_ROW)
	fileIds.push_back(stmt.getInt(0));

    return fileIds;
}

// =====================================================================================================================
void SqliteStorage::setFileMetadata(const zeppelin::library::File& file)
{
    if (!file.m_metadata)
	throw zeppelin::library::StorageException("file has no metadata");

    thread::BlockLock bl(m_mutex);

    int artistId = getArtistId(*file.m_metadata);
    int albumId = getAlbumId(artistId, *file.m_metadata);

    // set metadata informations on the files table
    {
	StatementHolder stmt(m_setFileMeta);
	stmt.bindIndex(1, artistId);
	stmt.bindIndex(2, albumId);
	stmt.bindInt(3, file.m_metadata->getLength());
	stmt.bindText(4, file.m_metadata->getTitle());
	stmt.bindInt(5, file.m_metadata->getYear());
	stmt.bindInt(6, file.m_metadata->getTrackIndex());
	stmt.bindText(7, file.m_metadata->getCodec());
	stmt.bindInt(8, file.m_metadata->getSampleRate());
	stmt.bindInt(9, file.m_metadata->getSampleSize());
	stmt.bindInt(10, file.m_metadata->getChannels());
	stmt.bindInt(11, file.m_id);
	stmt.step();
    }

    if (albumId != -1)
    {
	// add pictures
	for (auto& it : file.m_metadata->getPictures())
	{
	    StatementHolder stmt(m_addAlbumPicture);
	    stmt.bindInt(1, albumId);
	    switch (it.first)
	    {
		case Picture::FrontCover :
		    stmt.bindText(2, "frontcover");
		    break;
		case Picture::BackCover :
		    stmt.bindText(2, "backcover");
		    break;
	    }
	    stmt.bindText(3, it.second->getMimeType());
	    stmt.bindBlob(4, it.second->getData());
	    stmt.step();
	}
    }
}

// =====================================================================================================================
void SqliteStorage::updateFileMetadata(const zeppelin::library::File& file)
{
    if (!file.m_metadata)
	throw zeppelin::library::StorageException("file has no metadata");

    thread::BlockLock bl(m_mutex);

    int artistId = getArtistId(*file.m_metadata);
    int albumId = getAlbumId(artistId, *file.m_metadata);

    StatementHolder stmt(m_updateFileMeta);

    stmt.bindIndex(1, artistId);
    stmt.bindIndex(2, albumId);
    stmt.bindText(3, file.m_metadata->getTitle());
    stmt.bindInt(4, file.m_metadata->getYear());
    stmt.bindInt(5, file.m_metadata->getTrackIndex());
    stmt.bindInt(6, file.m_id);

    stmt.step();
}

// =====================================================================================================================
std::vector<std::shared_ptr<zeppelin::library::Artist>> SqliteStorage::getArtists(const std::vector<int>& ids)
{
    std::vector<std::shared_ptr<zeppelin::library::Artist>> artists;

    std::ostringstream query;
    query << "SELECT artists.id, artists.name, COUNT(DISTINCT files.album_id) ";
    query << "FROM artists LEFT JOIN files ON artists.id = files.artist_id ";
    if (!ids.empty())
    {
	query << "WHERE artists.id IN (";
	serializeIntList(query, ids);
	query << ") ";
    }
    query << "GROUP BY files.artist_id";

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	std::shared_ptr<zeppelin::library::Artist> artist = std::make_shared<zeppelin::library::Artist>(
	    stmt.getInt(0),
	    stmt.getText(1),
	    stmt.getInt(2));
	artists.push_back(artist);
    }

    return artists;
}

// =====================================================================================================================
std::vector<int> SqliteStorage::getAlbumIdsByArtist(int artistId)
{
    std::vector<int> ids;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_getAlbumIdsByArtist);
    stmt.bindInt(1, artistId);

    while (stmt.step() == SQLITE_ROW)
	ids.push_back(stmt.getInt(0));

    return ids;
}

// =====================================================================================================================
std::vector<std::shared_ptr<zeppelin::library::Album>> SqliteStorage::getAlbums(const std::vector<int>& ids)
{
    std::vector<std::shared_ptr<zeppelin::library::Album>> albums;

    std::ostringstream query;
    query << "SELECT albums.id, albums.name, albums.artist_id, COUNT(files.id) ";
    query << "FROM albums LEFT JOIN files ON albums.id = files.album_id ";
    if (!ids.empty())
    {
	query << "WHERE albums.id IN (";
	serializeIntList(query, ids);
	query << ") ";
    }
    query << "GROUP BY albums.id";

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	std::shared_ptr<zeppelin::library::Album> album = std::make_shared<zeppelin::library::Album>(
	    stmt.getInt(0),
	    stmt.getText(1),
	    stmt.getInt(2),
	    stmt.getInt(3));
	albums.push_back(album);
    }

    return albums;
}

// =====================================================================================================================
std::map<int, std::map<Picture::Type, std::shared_ptr<Picture>>> SqliteStorage::getPicturesOfAlbums(const std::vector<int>& ids)
{
    std::map<int, std::map<Picture::Type, std::shared_ptr<Picture>>> result;

    if (ids.empty())
	return result;

    std::ostringstream query;
    query << "SELECT album_id, type, mimetype, data FROM album_pictures WHERE album_id IN (";
    serializeIntList(query, ids);
    query << ")";

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	std::map<Picture::Type, std::shared_ptr<Picture>>& pictures = result[stmt.getInt(0) /* album ID */];

	std::vector<unsigned char> data;
	stmt.getBlob(3, data);

	std::shared_ptr<Picture> picture = std::make_shared<Picture>(stmt.getText(2), std::move(data));

	std::string type = stmt.getText(1);

	if (type == "frontcover")
	    pictures[Picture::FrontCover] = picture;
	else if (type == "backcover")
	    pictures[Picture::BackCover] = picture;
    }

    return result;
}

// =====================================================================================================================
int SqliteStorage::createPlaylist(const std::string& name)
{
    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_createPlaylist);
    stmt.bindText(1, name);
    stmt.step();

    return sqlite3_last_insert_rowid(m_db);
}

// =====================================================================================================================
void SqliteStorage::deletePlaylist(int id)
{
    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_deletePlaylist);
    stmt.bindInt(1, id);
    stmt.step();
}

// =====================================================================================================================
int SqliteStorage::addPlaylistItem(int id, const std::string& type, int itemId)
{
    if (type != "album" && type != "file" && type != "directory")
	return -1;

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_addPlaylistItem);
    stmt.bindInt(1, id);
    stmt.bindText(2, type);
    stmt.bindInt(3, itemId);
    stmt.step();

    return sqlite3_last_insert_rowid(m_db);
}

// =====================================================================================================================
void SqliteStorage::deletePlaylistItem(int id)
{
    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_deletePlaylistItem);
    stmt.bindInt(1, id);
    stmt.step();
}

// =====================================================================================================================
std::vector<std::shared_ptr<zeppelin::library::Playlist>> SqliteStorage::getPlaylists(const std::vector<int>& ids)
{
    std::vector<std::shared_ptr<zeppelin::library::Playlist>> playlists;

    std::ostringstream query;
    query << "SELECT playlists.id, playlists.name, playlist_items.id, playlist_items.type, playlist_items.item_id ";
    query << "FROM playlists LEFT JOIN playlist_items ON playlists.id = playlist_items.playlist_id ";
    if (!ids.empty())
    {
	query << "WHERE playlists.id IN (";
	serializeIntList(query, ids);
	query << ") ";
    }
    // order by playlist id to help the following algorithm to create the result ...
    query << "ORDER BY playlists.id";

    thread::BlockLock bl(m_mutex);

    StatementHolder stmt(m_db, query.str());

    while (stmt.step() == SQLITE_ROW)
    {
	int id = stmt.getInt(0);

	if (playlists.empty() || playlists.back()->m_id != id)
	{
	    std::shared_ptr<zeppelin::library::Playlist> playlist = std::make_shared<zeppelin::library::Playlist>();
	    playlist->m_id = id;
	    playlist->m_name = stmt.getText(1);
	    playlists.push_back(playlist);
	}
	//in case of empty playlist col 2 is null
	if (!stmt.isNull(2))
	    playlists.back()->m_items.push_back({stmt.getInt(2), stmt.getText(3), stmt.getInt(4)});
    }

    return playlists;
}

// =====================================================================================================================
void SqliteStorage::execute(const std::string& sql)
{
    char* error;

    if (sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &error) != SQLITE_OK)
	throw zeppelin::library::StorageException("unable to execute query");
}

// =====================================================================================================================
void SqliteStorage::prepareStatement(sqlite3_stmt** stmt, const std::string& sql)
{
    if (sqlite3_prepare_v2(m_db, sql.c_str(), sql.length() + 1, stmt, NULL) != SQLITE_OK)
	throw zeppelin::library::StorageException("unable to prepare statement: " + sql);
}

// =====================================================================================================================
int SqliteStorage::getFileIdByPath(const std::string& path, const std::string& name)
{
    StatementHolder stmt(m_getFileByPath);
    stmt.bindText(1, path);
    stmt.bindText(2, name);

    if (stmt.step() == SQLITE_ROW)
	return stmt.getInt(0);

    return -1;
}

// =====================================================================================================================
int SqliteStorage::getArtistId(const zeppelin::library::Metadata& metadata)
{
    if (metadata.getArtist().empty())
	return -1;

    // add artist
    {
	StatementHolder stmt(m_addArtist);
	stmt.bindText(1, metadata.getArtist());
	if (stmt.step() != SQLITE_DONE)
	    throw zeppelin::library::StorageException("unable to insert artist");
    }

    // get artist
    {
	StatementHolder stmt(m_getArtistIdByName);
	stmt.bindText(1, metadata.getArtist());
	if (stmt.step() != SQLITE_ROW)
	    throw zeppelin::library::StorageException("unable to get artist after inserting!");
	return stmt.getInt(0);
    }
}

// =====================================================================================================================
int SqliteStorage::getAlbumId(int artistId, const zeppelin::library::Metadata& metadata)
{
    if (metadata.getAlbum().empty())
	return -1;

    // add album
    {
	StatementHolder stmt(m_addAlbum);
	stmt.bindIndex(1, artistId);
	stmt.bindText(2, metadata.getAlbum());
	if (stmt.step() != SQLITE_DONE)
	    throw zeppelin::library::StorageException("unable to insert album");
    }

    // get album
    {
	StatementHolder stmt(m_getAlbumIdByName);
	stmt.bindIndex(1, artistId);
	stmt.bindText(2, metadata.getAlbum());
	if (stmt.step() != SQLITE_ROW)
	    throw zeppelin::library::StorageException("unable to get album after inserting!");
	return stmt.getInt(0);
    }
}

// =====================================================================================================================
void SqliteStorage::serializeIntList(std::ostringstream& stream, const std::vector<int>& list)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
	stream << list[i];

	if (i != list.size() - 1)
	    stream << ",";
    }
}

// =====================================================================================================================
SqliteStorage::StatementHolder::StatementHolder(sqlite3* db, const std::string& query)
{
    if (sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &m_stmt, NULL) != SQLITE_OK)
	throw zeppelin::library::StorageException("unable to prepare statement: " + query);

    m_finalize = true;
}

// =====================================================================================================================
SqliteStorage::StatementHolder::StatementHolder(sqlite3_stmt* stmt)
    : m_finalize(false),
      m_stmt(stmt)
{
}

// =====================================================================================================================
SqliteStorage::StatementHolder::~StatementHolder()
{
    if (m_finalize)
	sqlite3_finalize(m_stmt);
    else
	sqlite3_reset(m_stmt);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindNull(int col)
{
    sqlite3_bind_null(m_stmt, col);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindInt(int col, int value)
{
    sqlite3_bind_int(m_stmt, col, value);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindInt64(int col, int64_t value)
{
    sqlite3_bind_int64(m_stmt, col, value);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindText(int col, const std::string& value)
{
    sqlite3_bind_text(m_stmt, col, value.c_str(), value.length(), SQLITE_TRANSIENT);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindBlob(int col, const std::vector<unsigned char>& data)
{
    sqlite3_bind_blob(m_stmt, col, &data[0], data.size(), SQLITE_TRANSIENT);
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::bindIndex(int col, int value)
{
    if (value == -1)
	sqlite3_bind_null(m_stmt, col);
    else
	sqlite3_bind_int(m_stmt, col, value);
}

// =====================================================================================================================
int SqliteStorage::StatementHolder::step()
{
    return sqlite3_step(m_stmt);
}

// =====================================================================================================================
int SqliteStorage::StatementHolder::columnCount()
{
    return sqlite3_column_count(m_stmt);
}

// =====================================================================================================================
std::string SqliteStorage::StatementHolder::tableName(int col)
{
    return sqlite3_column_table_name(m_stmt, col);
}

// =====================================================================================================================
std::string SqliteStorage::StatementHolder::columnName(int col)
{
    return sqlite3_column_name(m_stmt, col);
}

// =====================================================================================================================
int SqliteStorage::StatementHolder::columnType(int col)
{
    return sqlite3_column_type(m_stmt, col);
}

// =====================================================================================================================
int SqliteStorage::StatementHolder::getInt(int col)
{
    return sqlite3_column_int(m_stmt, col);
}

// =====================================================================================================================
int64_t SqliteStorage::StatementHolder::getInt64(int col)
{
    return sqlite3_column_int64(m_stmt, col);
}

// =====================================================================================================================
std::string SqliteStorage::StatementHolder::getText(int col)
{
    const char* s = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, col));

    if (!s)
	return "";

    return s;
}

// =====================================================================================================================
void SqliteStorage::StatementHolder::getBlob(int col, std::vector<unsigned char>& data)
{
    data.resize(sqlite3_column_bytes(m_stmt, col));
    memcpy(&data[0], sqlite3_column_blob(m_stmt, col), data.size());
}

// =====================================================================================================================
bool SqliteStorage::StatementHolder::isNull(int col)
{
	return sqlite3_column_type(m_stmt, col) == SQLITE_NULL;
}
