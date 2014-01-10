#ifndef FILESERVER_FILESERVER_H_INCLUDED
#define FILESERVER_FILESERVER_H_INCLUDED

#include <plugins/http-server/httpserver.h>

#include <plugin/plugin.h>

class FileServer : public plugin::Plugin
{
    public:
	std::string getName() const override
	{ return "file-server"; }

	void start(const Json::Value& config, plugin::PluginManager& pm) override;
	void stop() override;

    private:
	std::unique_ptr<httpserver::HttpResponse> processRequest(const httpserver::HttpRequest& request);

    private:
	std::string m_documentRoot;
};

#endif
