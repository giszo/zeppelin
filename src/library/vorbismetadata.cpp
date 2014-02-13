#include "vorbismetadata.h"

#include <zeppelin/logger.h>

#include <boost/lexical_cast.hpp>

using library::VorbisMetadata;

// =====================================================================================================================
VorbisMetadata::VorbisMetadata(const std::string& codec)
    : Metadata(codec)
{
}

// =====================================================================================================================
void VorbisMetadata::setVorbisComment(const std::string& vc)
{
    std::string::size_type p = vc.find('=');

    if (p == std::string::npos)
    {
	LOG("vorbismetadata: invalid Vorbis comment");
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
