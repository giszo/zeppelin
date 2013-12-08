#include "basecodec.h"
#include "mp3.h"

#include <utils/stringutils.h>

using codec::BaseCodec;

// =====================================================================================================================
std::shared_ptr<BaseCodec> BaseCodec::openFile(const std::string& file)
{
    std::shared_ptr<BaseCodec> codec;

    if (utils::StringUtils::endsWith(file, ".mp3"))
	codec.reset(new Mp3());

    if (!codec)
	return nullptr;

    try
    {
	codec->open(file);
    }
    catch (const CodecException& e)
    {
	return nullptr;
    }

    return codec;
}
