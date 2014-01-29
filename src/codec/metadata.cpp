#include "metadata.h"

#include <zeppelin/logger.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

using codec::Metadata;

// =====================================================================================================================
Metadata::Metadata()
    : m_rate(-1),
      m_channels(-1),
      m_samples(0),
      m_year(0),
      m_trackIndex(0)
{
}

// =====================================================================================================================
const std::string& Metadata::getArtist() const
{
    return m_artist;
}

// =====================================================================================================================
const std::string& Metadata::getAlbum() const
{
    return m_album;
}

// =====================================================================================================================
const std::string& Metadata::getTitle() const
{
    return m_title;
}

// =====================================================================================================================
void Metadata::setArtist(const std::string& artist)
{
    m_artist = artist;
    boost::algorithm::trim(m_artist);
}

// =====================================================================================================================
void Metadata::setAlbum(const std::string& album)
{
    m_album = album;
    boost::algorithm::trim(m_album);
}

// =====================================================================================================================
void Metadata::setTitle(const std::string& title)
{
    m_title = title;
    boost::algorithm::trim(m_title);
}

// =====================================================================================================================
void Metadata::setVorbisComment(const std::string& vc)
{
    std::string::size_type p = vc.find('=');

    if (p == std::string::npos)
    {
	LOG("metadata: invalid Vorbis comment");
	return;
    }

    std::string key = vc.substr(0, p);
    std::string value = vc.substr(p + 1);

    if (key == "ARTIST")
	setArtist(value);
    else if (key == "ALBUM")
	setAlbum(value);
    else if (key == "TITLE")
	setTitle(value);
    else if (key == "DATE")
    {
	try
	{
	    m_year = boost::lexical_cast<int>(value);
	}
	catch (const boost::bad_lexical_cast&)
	{
	}
    }
    else if (key == "TRACKNUMBER")
    {
	try
	{
	    m_trackIndex = boost::lexical_cast<int>(value);
	}
	catch (const boost::bad_lexical_cast&)
	{
	}
    }
}
