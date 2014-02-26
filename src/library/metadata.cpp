/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#include <zeppelin/library/metadata.h>

#include <boost/algorithm/string/trim.hpp>

using zeppelin::library::Metadata;

// =====================================================================================================================
Metadata::Metadata(const std::string& codec)
    : m_year(0),
      m_trackIndex(0),
      m_length(0),
      m_codec(codec),
      m_sampleRate(0),
      m_sampleSize(0)
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
int Metadata::getYear() const
{
    return m_year;
}

// =====================================================================================================================
int Metadata::getTrackIndex() const
{
    return m_trackIndex;
}

// =====================================================================================================================
const std::string& Metadata::getCodec() const
{
    return m_codec;
}

// =====================================================================================================================
int Metadata::getLength() const
{
    return m_length;
}

// =====================================================================================================================
int Metadata::getChannels() const
{
    return m_channels;
}

// =====================================================================================================================
int Metadata::getSampleRate() const
{
    return m_sampleRate;
}

// =====================================================================================================================
int Metadata::getSampleSize() const
{
    return m_sampleSize;
}

// =====================================================================================================================
auto Metadata::getPictures() const -> const std::map<Picture::Type, std::shared_ptr<Picture>>&
{
    return m_pictures;
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
void Metadata::setYear(int year)
{
    m_year = year;
}

// =====================================================================================================================
void Metadata::setTrackIndex(int trackIndex)
{
    m_trackIndex = trackIndex;
}

// =====================================================================================================================
void Metadata::setFormat(int channels, int sampleRate, int sampleSize)
{
    m_channels = channels;
    m_sampleRate = sampleRate;
    m_sampleSize = sampleSize;
}

// =====================================================================================================================
void Metadata::setLength(int length)
{
    m_length = length;
}

// =====================================================================================================================
void Metadata::addPicture(Picture::Type type, const std::shared_ptr<Picture>& picture)
{
    m_pictures[type] = picture;
}
