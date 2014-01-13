#include "basecodec.h"
#include "mp3.h"
#include "flac.h"

#include <utils/stringutils.h>

using codec::BaseCodec;

// =====================================================================================================================
std::shared_ptr<BaseCodec> BaseCodec::create(const std::string& file)
{
    if (utils::StringUtils::endsWith(file, ".mp3"))
	return std::make_shared<Mp3>(file);
    else if (utils::StringUtils::endsWith(file, ".flac"))
	return std::make_shared<Flac>(file);

    return nullptr;
}
