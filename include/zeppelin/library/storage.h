#ifndef ZEPPELIN_LIBRARY_STORAGE_H_INCLUDED
#define ZEPPELIN_LIBRARY_STORAGE_H_INCLUDED

#include <zeppelin/library/file.h>
#include <zeppelin/library/directory.h>
#include <zeppelin/library/artist.h>
#include <zeppelin/library/album.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

namespace zeppelin
{
namespace library
{

class FileNotFoundException : public std::runtime_error
{
    public:
	FileNotFoundException(const std::string& error)
	    : runtime_error(error)
	{}
};

class StorageException : public std::runtime_error
{
    public:
	StorageException(const std::string& error)
	    : runtime_error(error)
	{}
};

struct Statistics
{
    // number of artists
    int m_numOfArtists;
    // number of albums
    int m_numOfAlbums;
    // number of files
    int m_numOfFiles;
    // sum of song lengths
    int m_sumOfSongLength;
};

class Storage
{
    public:
	virtual ~Storage()
	{}

	// returns statistics about the music library
	virtual Statistics getStatistics() = 0;

	/// returns the directory structure associated to the given ID
	virtual std::shared_ptr<Directory> getDirectory(int id) = 0;
	/// ensures that the given directory with the parent exists in the database and returns its ID
	virtual int ensureDirectory(const std::string& name, int parentId) = 0;
	/// lists the subdirectories of the given directory
	virtual std::vector<std::shared_ptr<Directory>> listSubdirectories(int id) = 0;

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
	/// returns the files of the given directory
	virtual std::vector<std::shared_ptr<File>> getFilesOfDirectory(int directoryId) = 0;

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
}

#endif
