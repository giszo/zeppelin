#include "metadata.h"

#include <boost/algorithm/string/trim.hpp>

using codec::Metadata;

// =====================================================================================================================
Metadata::Metadata()
    : m_type(UNKNOWN),
      m_rate(-1),
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
