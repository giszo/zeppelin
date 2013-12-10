#ifndef LIBRARY_STORAGE_H_INCLUDED
#define LIBRARY_STORAGE_H_INCLUDED

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
    Artist(int id, const std::string& name)
	: m_id(id),
	  m_name(name)
    {}

    int m_id;
    std::string m_name;
};

struct Album
{
    Album(int id, const std::string& name)
	: m_id(id),
	  m_name(name),
	  m_artist(-1, "")
    {}

    Album(int id, const std::string& name, int artistId, const std::string& artistName)
	: m_id(id),
	  m_name(name),
	  m_artist(artistId, artistName)
    {}

    int m_id;
    std::string m_name;
    Artist m_artist;
};

struct File
{
    File(int id, const std::string& path, const std::string& name)
	: m_id(id),
	  m_path(path),
	  m_name(name),
	  m_length(0),
	  m_year(0)
    {}

    File(int id, const std::string& path, const std::string& name,
	 int length, const std::string& artist, const std::string& album, const std::string& title, int year)
	: m_id(id),
	  m_path(path),
	  m_name(name),
	  m_length(length),
	  m_artist(artist),
	  m_album(album),
	  m_title(title),
	  m_year(year)
    {}

    int m_id;
    std::string m_path;
    std::string m_name;

    /// the length of the music file in seconds
    int m_length;

    std::string m_artist;
    std::string m_album;
    std::string m_title;
    int m_year;
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

	/// adds a new file to the storage
	virtual void addFile(File& file) = 0;

	/// returns the file informations associated to the given ID
	virtual std::shared_ptr<File> getFile(int id) = 0;
	/// returns all of the files from the underlying storage
	virtual std::vector<std::shared_ptr<File>> getFiles() = 0;
	/// retuns the given amount of files at most from the library having no metadata yet
	virtual std::vector<std::shared_ptr<File>> getFilesWithoutMetadata(int amount) = 0;

	/// updates the metadate of the given file.
	virtual void updateFileMetadata(const File& file) = 0;

	/// returns the available artists from the database
	virtual std::vector<std::shared_ptr<Artist>> getArtists() = 0;

	/// returns the available albums from the database (without artist filtering)
	virtual std::vector<std::shared_ptr<Album>> getAlbums() = 0;
	/// returns the available albums associated to the given artist
	virtual std::vector<std::shared_ptr<Album>> getAlbumsByArtist(int artistId) = 0;
};

}

#endif
