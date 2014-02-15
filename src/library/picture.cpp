#include <zeppelin/library/picture.h>

#include <cstring>

using zeppelin::library::Picture;

// =====================================================================================================================
Picture::Picture(const std::string& mimeType, const unsigned char* data, size_t size)
    : m_mimeType(mimeType)
{
    m_data.resize(size);
    memcpy(&m_data[0], data, size);
}

// =====================================================================================================================
Picture::Picture(const std::string& mimeType, std::vector<unsigned char>&& data)
    : m_mimeType(mimeType),
      m_data(std::move(data))
{
}

// =====================================================================================================================
const std::string& Picture::getMimeType() const
{
    return m_mimeType;
}

// =====================================================================================================================
const std::vector<unsigned char>& Picture::getData() const
{
    return m_data;
}
