#ifndef HTTPSERVER_REQUEST_H_INCLUDED
#define HTTPSERVER_REQUEST_H_INCLUDED

#include <plugins/http-server/httpserver.h>

#include <microhttpd.h>

class MHDHttpRequest : public httpserver::HttpRequest
{
    public:
	MHDHttpRequest(MHD_Connection* connection,
		       const std::string& method,
		       const std::string& url);

	MHD_Connection* getConnection() const
	{ return m_connection; }

	const std::string& getMethod() const override
	{ return m_method; }
	const std::string& getUrl() const override
	{ return m_url; }

	const std::string& getData() const override
	{ return m_data; }

	std::unique_ptr<httpserver::HttpResponse> createBufferedResponse(int, const std::string&) const override;
	std::unique_ptr<httpserver::HttpResponse> createFileResponse(int) const override;

	void processUploadData(const char* data, size_t size);

    private:
	MHD_Connection* m_connection;

	std::string m_method;
	std::string m_url;

	std::string m_data;
};

#endif
