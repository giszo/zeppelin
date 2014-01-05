#ifndef HTTPSERVER_REQUEST_H_INCLUDED
#define HTTPSERVER_REQUEST_H_INCLUDED

#include <plugins/http-server/httpserver.h>

class MHDHttpRequest : public httpserver::HttpRequest
{
    public:
	MHDHttpRequest(const std::string& method, const std::string& url);

	const std::string& getMethod() const override
	{ return m_method; }
	const std::string& getUrl() const override
	{ return m_url; }

	const std::string& getData() const override
	{ return m_data; }

	void processUploadData(const char* data, size_t size);

    private:
	std::string m_method;
	std::string m_url;

	std::string m_data;
};

#endif
