#include "codecmanager.h"
#include "mp3.h"
#include "flac.h"
#include "vorbis.h"
#include "wavpack.h"

#include <zeppelin/logger.h>

using codec::CodecManager;

CodecManager::CodecMap CodecManager::s_codecs = {
    {"mp3",  [](const std::string& file) { return std::make_shared<codec::Mp3>(file); }},
    {"flac", [](const std::string& file) { return std::make_shared<codec::Flac>(file); }},
    {"ogg",  [](const std::string& file) { return std::make_shared<codec::Vorbis>(file); }},
    {"wv",   [](const std::string& file) { return std::make_shared<codec::WavPack>(file); }}
};

// =====================================================================================================================
std::shared_ptr<codec::BaseCodec> CodecManager::create(const std::string& file) const
{
    auto it = findCodec(file);

    if (it == s_codecs.end())
	return nullptr;

    return it->second(file);
}

// =====================================================================================================================
bool CodecManager::isMediaFile(const std::string& file) const
{
    return findCodec(file) != s_codecs.end();
}

// =====================================================================================================================
CodecManager::CodecMap::const_iterator CodecManager::findCodec(const std::string& file) const
{
    std::string::size_type p = file.rfind('.');

    if (p == std::string::npos)
	return s_codecs.end();

    return s_codecs.find(file.substr(p + 1));
}
