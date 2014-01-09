#ifndef LIBRARY_STORAGE_H_INCLUDED
#define LIBRARY_STORAGE_H_INCLUDED

#include <codec/type.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

namespace library
{

class FileNotFoundException : public std::runtime_error
{
    public:
	FileNotFoundException(const std::string& error)
	    : runtime_error(error)
	{}
};

struct Artist
{
    Artist(int id, const std::string& name, int albums)
	: m_id(id),
	  m_name(name),
	  m_albums(albums)
    {}

    int m_id;
    std::string m_name;
    int m_albums;
};

struct Album
{
    Album(int id, const std::string& name, int artist, int songs, int length)
	: m_id(id),
	  m_name(name),
	  m_artist(artist),
	  m_songs(songs),
	  m_length(length)
    {}

    int m_id;
    std::string m_name;
    int m_artist;
    /// number of songs in this album
    int m_songs;
    /// length of the album in seconds
    int m_length;
};

struct File
{
    File(int id)
	: m_id(id),
	  m_size(0),
	  m_length(0),
	  m_artistId(0),
	  m_albumId(0),
	  m_year(0),
	  m_trackIndex(0),
	  m_type(codec::UNKNOWN),
	  m_samplingRate(0)
    {}

    File(int id, const std::string& path, const std::string& name, int64_t size)
	: m_id(id),
	  m_path(path),
	  m_name(name),
	  m_size(size),
	  m_length(0),
	  m_artistId(0),
	  m_albumId(0),
	  m_year(0),
	  m_trackIndex(0),
	  m_type(codec::UNKNOWN),
	  m_samplingRate(0)
    {}

    File(int id, const std::string& path, const std::string& name, int64_t size,
	 int length, const std::string& artist, const std::string& album, const std::string& title, int year, int trackIndex,
	 codec::Type type, int samplingRate)
	: m_id(id),
	  m_path(path),
	  m_name(name),
	  m_length(length),
	  m_artist(artist),
	  m_album(album),
	  m_title(title),
	  m_year(year),
	  m_trackIndex(trackIndex),
	  m_type(type),
	  m_samplingRate(samplingRate)
    {}

    File(int id, const std::string& path, const std::string& name, int64_t size,
	 int length, int artistId, int albumId, const std::string& title, int year, int trackIndex,
	 codec::Type type, int samplingRate)
	: m_id(id),
	  m_path(path),
	  m_name(name),
	  m_length(length),
	  m_artistId(artistId),
	  m_albumId(albumId),
	  m_title(title),
	  m_year(year),
	  m_trackIndex(trackIndex),
	  m_type(type),
	  m_samplingRate(samplingRate)
    {}

    int m_id;
    std::string m_path;
    std::string m_name;
    // size of the file in bytes
    int64_t m_size;

    /// the length of the music file in seconds
    int m_length;

    std::string m_artist;
    std::string m_album;
    int m_artistId;
    int m_albumId;
    std::string m_title;
    int m_year;
    int m_trackIndex;

    // the type of the file (mp3, flac, etc.)
    codec::Type m_type;
    // sampling rate of the file (44100Hz, 48000Hz, etc.)
    int m_samplingRate;
};

class StorageException : public std::runtime_error
{
    public:
	StorageException(const std::string& error)
	    : runtime_error(error)
	{}
};

class Storage
{
    public:
	virtual ~Storage()
	{}

	/**
	 * Adds a new file to the storage.
	 * @return true if this is a new file
	 */
	virtual bool addFile(File& file) = 0;

	/// clears the mark flag from all files
	virtual void clearMark() = 0;
	/// deletes those files from the database having no mark
	virtual void deleteNonMarked() = 0;

	/// returns the file informations associated to the given ID
	virtual std::shared_ptr<File> getFile(int id) = 0;
	/// returns the given amount of files at most from the library having no metadata yet
	virtual std::vector<std::shared_ptr<File>> getFilesWithoutMetadata() = 0;
	/// returns the files of the given artist
	virtual std::vector<std::shared_ptr<File>> getFilesOfArtist(int artistId) = 0;
	/// returns the files of the given album
	virtual std::vector<std::shared_ptr<File>> getFilesOfAlbum(int albumId) = 0;

	// Sets all metadata related fields of the file including length, sampling rate, etc.
	virtual void setFileMetadata(const File& file) = 0;
	// Updates only those parts of the metadata that can be edited by the user (artist, album, etc.)
	virtual void updateFileMetadata(const File& file) = 0;

	/// returns the available artists from the database
	virtual std::vector<std::shared_ptr<Artist>> getArtists() = 0;

	/// returns the album associated to the given ID
	virtual std::shared_ptr<Album> getAlbum(int id) = 0;
	/// returns the available albums from the database (without artist filtering)
	virtual std::vector<std::shared_ptr<Album>> getAlbums() = 0;
	/// returns the available albums associated to the given artist
	virtual std::vector<std::shared_ptr<Album>> getAlbumsByArtist(int artistId) = 0;
};

}

#endif
