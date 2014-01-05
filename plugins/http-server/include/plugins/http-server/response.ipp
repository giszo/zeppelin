inline HttpResponse::HttpResponse(int status, const std::string& data)
	    : m_status(status),
	      m_data(data)
	{}

// =====================================================================================================================
inline void HttpResponse::addHeader(const std::string& key, const std::string& value)
{
    m_headers[key] = value;
}

// =====================================================================================================================
inline int HttpResponse::getStatus() const
{
    return m_status;
}

// =====================================================================================================================
inline const std::string& HttpResponse::getData() const
{
    return m_data;
}

// =====================================================================================================================
inline const std::unordered_map<std::string, std::string>& HttpResponse::getHeaders() const
{
    return m_headers;
}
