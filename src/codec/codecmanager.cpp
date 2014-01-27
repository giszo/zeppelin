#include "codecmanager.h"

#include <zeppelin/logger.h>

using codec::CodecManager;

// =====================================================================================================================
void CodecManager::registerCodec(const std::string& type, const CreateCodec& func)
{
    m_codecs[type] = func;
}

// =====================================================================================================================
std::shared_ptr<codec::BaseCodec> CodecManager::create(const std::string& file) const
{
    auto it = findCodec(file);

    if (it == m_codecs.end())
	return nullptr;

    return it->second(file);
}

// =====================================================================================================================
bool CodecManager::isMediaFile(const std::string& file) const
{
    return findCodec(file) != m_codecs.end();
}

// =====================================================================================================================
CodecManager::CodecMap::const_iterator CodecManager::findCodec(const std::string& file) const
{
    std::string::size_type p = file.rfind('.');

    if (p == std::string::npos)
	return m_codecs.end();

    return m_codecs.find(file.substr(p + 1));
}
