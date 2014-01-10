#ifndef HTTPSERVER_RESPONSE_H_INCLUDED
#define HTTPSERVER_RESPONSE_H_INCLUDED

#include <plugins/http-server/httpserver.h>

class MHDBufferedHttpResponse : public httpserver::BufferedHttpResponse
{
    public:
	MHDBufferedHttpResponse(int status,
				const std::string& data,
				const httpserver::HttpRequest& request);

	void addHeader(const std::string& key, const std::string& value) override;

	const httpserver::HttpRequest& getRequest() const override;
	int getStatus() const override;
	const std::unordered_map<std::string, std::string>& getHeaders() const override;

	void send(httpserver::HttpServer&) const override;

	const std::string& getBuffer() const override;

    private:
	int m_status;
	std::unordered_map<std::string, std::string> m_headers;

	std::string m_data;

	const httpserver::HttpRequest& m_request;
};

class MHDFileHttpResponse : public httpserver::FileHttpResponse
{
    public:
	MHDFileHttpResponse(int fd,
			    const httpserver::HttpRequest& request);

	void addHeader(const std::string& key, const std::string& value) override;

	const httpserver::HttpRequest& getRequest() const override;
	int getStatus() const override;
	const std::unordered_map<std::string, std::string>& getHeaders() const override;

	void send(httpserver::HttpServer&) const override;

	int getFd() const override;

    private:
	std::unordered_map<std::string, std::string> m_headers;

	int m_fd;

	const httpserver::HttpRequest& m_request;
};

#endif
