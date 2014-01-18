#ifndef PLUGINS_HTTPSERVER_HTTPSERVER_H_INCLUDED
#define PLUGINS_HTTPSERVER_HTTPSERVER_H_INCLUDED

#include <zeppelin/plugin/plugininterface.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <string>

#define HTTP_SERVER_VERSION 1

namespace httpserver
{

class HttpResponse;
class HttpServer;

class HttpRequest
{
    public:
	virtual ~HttpRequest()
	{}

	virtual const std::string& getMethod() const = 0;
	virtual const std::string& getUrl() const = 0;

	virtual const std::string& getData() const = 0;

	virtual std::unique_ptr<HttpResponse> createBufferedResponse(int, const std::string&) const = 0;
	virtual std::unique_ptr<HttpResponse> createFileResponse(int) const = 0;
};

class HttpResponse
{
    public:
	virtual ~HttpResponse()
	{}

	virtual void addHeader(const std::string& key, const std::string& value) = 0;

	virtual const HttpRequest& getRequest() const = 0;
	virtual int getStatus() const = 0;
	virtual const std::unordered_map<std::string, std::string>& getHeaders() const = 0;

	virtual void send(HttpServer&) const = 0;
};

class BufferedHttpResponse : public HttpResponse
{
    public:
	virtual const std::string& getBuffer() const = 0;
};

class FileHttpResponse : public HttpResponse
{
    public:
	virtual int getFd() const = 0;
};

class HttpServer : public zeppelin::plugin::PluginInterface
{
    public:
	typedef std::function<std::unique_ptr<HttpResponse> (const HttpRequest&)> Handler;

	int version() const override
	{ return HTTP_SERVER_VERSION; }

	virtual void registerHandler(const std::string& url, const Handler& handler) = 0;

	virtual void sendResponse(const BufferedHttpResponse&) = 0;
	virtual void sendResponse(const FileHttpResponse&) = 0;
};

}

#endif
