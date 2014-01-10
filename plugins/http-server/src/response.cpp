#include "response.h"

// =====================================================================================================================
MHDBufferedHttpResponse::MHDBufferedHttpResponse(int status,
						 const std::string& data,
						 const httpserver::HttpRequest& request)
    : m_status(status),
      m_data(data),
      m_request(request)
{
}

// =====================================================================================================================
void MHDBufferedHttpResponse::addHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

// =====================================================================================================================
const httpserver::HttpRequest& MHDBufferedHttpResponse::getRequest() const
{
    return m_request;
}

// =====================================================================================================================
int MHDBufferedHttpResponse::getStatus() const
{
    return m_status;
}

// =====================================================================================================================
const std::unordered_map<std::string, std::string>& MHDBufferedHttpResponse::getHeaders() const
{
    return m_headers;
}

// =====================================================================================================================
void MHDBufferedHttpResponse::send(httpserver::HttpServer& server) const
{
    server.sendResponse(*this);
}

// =====================================================================================================================
const std::string& MHDBufferedHttpResponse::getBuffer() const
{
    return m_data;
}

// =====================================================================================================================
MHDFileHttpResponse::MHDFileHttpResponse(int fd,
					 const httpserver::HttpRequest& request)
    : m_fd(fd),
      m_request(request)
{
}

// =====================================================================================================================
void MHDFileHttpResponse::addHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

// =====================================================================================================================
const httpserver::HttpRequest& MHDFileHttpResponse::getRequest() const
{
    return m_request;
}

// =====================================================================================================================
int MHDFileHttpResponse::getStatus() const
{
    return 200 /* for now ... :) */;
}

// =====================================================================================================================
const std::unordered_map<std::string, std::string>& MHDFileHttpResponse::getHeaders() const
{
    return m_headers;
}

// =====================================================================================================================
void MHDFileHttpResponse::send(httpserver::HttpServer& server) const
{
    server.sendResponse(*this);
}

// =====================================================================================================================
int MHDFileHttpResponse::getFd() const
{
    return m_fd;
}
