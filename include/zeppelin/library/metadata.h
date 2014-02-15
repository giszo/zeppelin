#ifndef ZEPPELIN_LIBRARY_METADATA_H_INCLUDED
#define ZEPPELIN_LIBRARY_METADATA_H_INCLUDED

#include "picture.h"

#include <string>
#include <map>
#include <memory>

namespace zeppelin
{
namespace library
{

class Metadata
{
    public:
	Metadata(const std::string& codec);

	const std::string& getArtist() const;
	const std::string& getAlbum() const;
	const std::string& getTitle() const;
	int getYear() const;
	int getTrackIndex() const;

	const std::string& getCodec() const;
	int getLength() const;
	int getChannels() const;
	int getSampleRate() const;
	int getSampleSize() const;

	const std::map<Picture::Type, std::shared_ptr<Picture>>& getPictures() const;

	void setArtist(const std::string& artist);
	void setAlbum(const std::string& album);
	void setTitle(const std::string& title);
	void setYear(int year);
	void setTrackIndex(int trackIndex);

	void setFormat(int channels, int sampleRate, int sampleSize);
	void setLength(int length);

	void addPicture(Picture::Type type, const std::shared_ptr<Picture>& picture);

    protected:
	// name of the artist
	std::string m_artist;
	// name of the album
	std::string m_album;
	// title of the song
	std::string m_title;
	// year of the song
	int m_year;
	// index of the song on the album
	int m_trackIndex;

	// the length of the song in seconds
	int m_length;
	// the audio codec of the file (mp3, flac, etc.)
	std::string m_codec;
	// the number of channels in the media
	int m_channels;
	// sampling rate of the file (44100Hz, 48000Hz, etc.)
	int m_sampleRate;
	// size of a sample in bits
	int m_sampleSize;

	std::map<Picture::Type, std::shared_ptr<Picture>> m_pictures;
};

}
}

#endif
