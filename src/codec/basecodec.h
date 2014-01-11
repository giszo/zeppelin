#ifndef CODEC_BASECODEC_H_INCLUDED
#define CODEC_BASECODEC_H_INCLUDED

#include "metadata.h"

#include <player/format.h>

#include <string>
#include <stdexcept>
#include <memory>

namespace codec
{

class CodecException : public std::runtime_error
{
    public:
	CodecException(const std::string& error) :
	    runtime_error(error)
	{}
};

class BaseCodec
{
    protected:
	BaseCodec(const std::string& file)
	    : m_file(file)
	{}

    public:
	virtual ~BaseCodec()
	{}

	virtual void open() = 0;

	// returns the format (sampling rate, channels, etc.) of the input
	virtual player::Format getFormat() const = 0;

	/**
	 * Decodes the next part of the media stream.
	 * @return false is returned at the end of the stream
	 */
	virtual bool decode(float*& samples, size_t& count) = 0;

	/// returns informations about the media
	virtual Metadata readMetadata() = 0;

	static std::shared_ptr<BaseCodec> create(const std::string& file);

    protected:
	std::string m_file;
};

}

#endif
