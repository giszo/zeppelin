#include "request.h"

// =====================================================================================================================
MHDHttpRequest::MHDHttpRequest(const std::string& method, const std::string& url)
    : m_method(method),
      m_url(url)
{
}

// =====================================================================================================================
void MHDHttpRequest::processUploadData(const char* data, size_t size)
{
    m_data.append(data, size);
}
