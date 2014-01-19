#include "basecodec.h"
#include "mp3.h"
#include "flac.h"

#include <utils/stringutils.h>

using codec::BaseCodec;

BaseCodec::CodecMap BaseCodec::s_codecs = {
    {"mp3",  [](const std::string& file) { return std::make_shared<codec::Mp3>(file); }},
    {"flac", [](const std::string& file) { return std::make_shared<codec::Flac>(file); }}
};

// =====================================================================================================================
bool BaseCodec::isMediaFile(const std::string& file)
{
    std::string::size_type p = file.rfind('.');

    if (p == std::string::npos)
	return nullptr;

    return s_codecs.find(file.substr(p + 1)) != s_codecs.end();
}

// =====================================================================================================================
std::shared_ptr<BaseCodec> BaseCodec::create(const std::string& file)
{
    std::string::size_type p = file.rfind('.');

    if (p == std::string::npos)
	return nullptr;

    auto it = s_codecs.find(file.substr(p + 1));

    if (it == s_codecs.end())
	return nullptr;

    return it->second(file);
}
