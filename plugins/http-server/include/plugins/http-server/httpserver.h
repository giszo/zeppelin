#ifndef PLUGINS_HTTPSERVER_HTTPSERVER_H_INCLUDED
#define PLUGINS_HTTPSERVER_HTTPSERVER_H_INCLUDED

#include <plugin/plugininterface.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>

#define HTTP_SERVER_VERSION 1

namespace httpserver
{

class HttpRequest
{
    public:
	virtual ~HttpRequest()
	{}

	virtual const std::string& getMethod() const = 0;
	virtual const std::string& getUrl() const = 0;

	virtual const std::string& getData() const = 0;
};

class HttpResponse
{
    public:
	HttpResponse(int status, const std::string& data);

	void addHeader(const std::string& key, const std::string& value);

	int getStatus() const;
	const std::string& getData() const;
	const std::unordered_map<std::string, std::string>& getHeaders() const;

    private:
	int m_status;
	std::string m_data;
	std::unordered_map<std::string, std::string> m_headers;
};

class HttpServer : public plugin::PluginInterface
{
    public:
	typedef std::function<std::unique_ptr<HttpResponse> (const HttpRequest&)> Handler;

	int version() const override
	{ return HTTP_SERVER_VERSION; }

	virtual void registerHandler(const std::string& url, const Handler& handler) = 0;
};

#include "response.ipp"

}

#endif
