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
