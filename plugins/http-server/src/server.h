#ifndef HTTPSERVER_SERVER_H_INCLUDED
#define HTTPSERVER_SERVER_H_INCLUDED

#include <plugins/http-server/httpserver.h>

#include <plugin/plugin.h>

#include <microhttpd.h>

#include <vector>
#include <stdexcept>

class Server : public plugin::Plugin,
	       public httpserver::HttpServer
{
    public:
	Server();
	~Server();

	std::string getName() const override
	{ return "http-server"; }

	void start(const Json::Value& config, plugin::PluginManager& pm) override;
	void stop() override;

	void registerHandler(const std::string& url, const Handler& handler) override;

	void sendResponse(const httpserver::BufferedHttpResponse&) override;
	void sendResponse(const httpserver::FileHttpResponse&) override;

    private:
	class HandlerNotFoundException : public std::runtime_error
	{
	    public:
		HandlerNotFoundException() : runtime_error("handler not found") {}
	};

	int requestHandler(MHD_Connection* connection,
			   const std::string& url,
			   const std::string& method,
			   const std::string& ver,
			   const char* uploadData,
			   size_t* uploadDataSize,
			   void** conCls);

	const Handler& lookupHandler(const std::string& url) const;

	void sendNotFound(MHD_Connection* connection);

	static int _requestHandler(void* cls,
				   MHD_Connection* connection,
				   const char* url,
				   const char* method,
				   const char* version,
				   const char* uploadData,
				   size_t* uploadDataSize,
				   void** conCls);

	static void _requestCompleted(void* cls,
				      MHD_Connection* connection,
				      void** conCls,
				      MHD_RequestTerminationCode toe);

    private:
	MHD_Daemon* m_daemon;

	struct HandlerItem
	{
	    std::string m_baseUrl;
	    Handler m_handler;
	};

	std::vector<HandlerItem> m_handlers;
};

#endif
