/**
 * This file is part of the Zeppelin music player project.
 * Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
 * See http://zeppelin-player.com for more details.
 */

#ifndef LIBRARY_VORBISMETADATA_H_INCLUDED
#define LIBRARY_VORBISMETADATA_H_INCLUDED

#include <zeppelin/library/metadata.h>

namespace library
{

class VorbisMetadata : public zeppelin::library::Metadata
{
    public:
	VorbisMetadata(const std::string& codec);

	void setVorbisComment(const std::string& vc);
};

}

#endif
