#ifndef CODEC_METADATA_H_INCLUDED
#define CODEC_METADATA_H_INCLUDED

#include <string>

namespace codec
{

class Metadata
{
    public:
	Metadata();

	const std::string& getArtist() const;
	const std::string& getAlbum() const;
	const std::string& getTitle() const;

	void setArtist(const std::string& artist);
	void setAlbum(const std::string& album);
	void setTitle(const std::string& title);

	void setVorbisComment(const std::string& vc);

    public:
	std::string m_codec;

	/// sampling rate
	int m_rate;
	/// number of channels
	int m_channels;
	/// size of a sample (in bits)
	int m_sampleSize;
	/// the number of samples in the resource
	size_t m_samples;

	int m_year;
	int m_trackIndex;

    private:
	std::string m_artist;
	std::string m_album;
	std::string m_title;
};

}

#endif
