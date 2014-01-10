#include "request.h"
#include "response.h"

// =====================================================================================================================
MHDHttpRequest::MHDHttpRequest(MHD_Connection* connection,
			       const std::string& method,
			       const std::string& url)
    : m_connection(connection),
      m_method(method),
      m_url(url)
{
}

// =====================================================================================================================
std::unique_ptr<httpserver::HttpResponse> MHDHttpRequest::createBufferedResponse(int status, const std::string& data) const
{
    return std::unique_ptr<httpserver::HttpResponse>(new MHDBufferedHttpResponse(status, data, *this));
}

// =====================================================================================================================
std::unique_ptr<httpserver::HttpResponse> MHDHttpRequest::createFileResponse(int fd) const
{
    return std::unique_ptr<httpserver::HttpResponse>(new MHDFileHttpResponse(fd, *this));
}

// =====================================================================================================================
void MHDHttpRequest::processUploadData(const char* data, size_t size)
{
    m_data.append(data, size);
}
