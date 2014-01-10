#include "fileserver.h"

#include <plugin/pluginmanager.h>
#include <utils/makestring.h>
#include <logger.h>

#include <fcntl.h>

// =====================================================================================================================
void FileServer::start(const Json::Value& config, plugin::PluginManager& pm)
{
    if (!config.isMember("document-root") || !config["document-root"].isString())
    {
	LOG("file-server: document-root not configured properly");
	return;
    }

    m_documentRoot = config["document-root"].asString();

    try
    {
	httpserver::HttpServer& httpServer = static_cast<httpserver::HttpServer&>(pm.getInterface("http-server"));

	if (httpServer.version() != HTTP_SERVER_VERSION)
	{
	    LOG("jsonrpc-remote: invalid http-server plugin version!");
	    return;
	}

	httpServer.registerHandler("/", std::bind(&FileServer::processRequest, this, std::placeholders::_1));
    }
    catch (const plugin::PluginInterfaceNotFoundException& e)
    {
	LOG("jsonrpc-remote: http-server interface not found");
    }

}

// =====================================================================================================================
void FileServer::stop()
{
}

// =====================================================================================================================
std::unique_ptr<httpserver::HttpResponse> FileServer::processRequest(const httpserver::HttpRequest& request)
{
    std::string file = m_documentRoot + "/" + request.getUrl();

    if (request.getUrl() == "/")
	file += "index.html";

    int fd = open(file.c_str(), O_RDONLY);

    if (fd < 0)
	return nullptr;

    return std::unique_ptr<httpserver::HttpResponse>(request.createFileResponse(fd));
}
