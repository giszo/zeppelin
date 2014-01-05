#ifndef HTTPSERVER_SERVER_H_INCLUDED
#define HTTPSERVER_SERVER_H_INCLUDED

#include <plugins/http-server/httpserver.h>

#include <plugin/plugin.h>

#include <microhttpd.h>

#include <unordered_map>

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

    private:
	int requestHandler(MHD_Connection* connection,
			   const std::string& url,
			   const std::string& method,
			   const std::string& ver,
			   const char* uploadData,
			   size_t* uploadDataSize,
			   void** conCls);

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

	std::unordered_map<std::string, Handler> m_handlers;
};

#endif
