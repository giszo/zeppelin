#include "fileserver.h"

#include <plugin/pluginmanager.h>
#include <utils/makestring.h>
#include <logger.h>

#include <fcntl.h>

const std::string FileServer::s_contentType = "Content-Type";

std::map<std::string, std::string> FileServer::s_contentTypeMap = {
    {"html", "text/html"},
    {"js",   "text/javascript"},
    {"css",  "text/css"},
    {"json", "application/json"},
    {"png",  "image/png"},
    {"jpg",  "image/jpeg"},
    {"gif",  "image/gif"},
    {"ico",  "image/x-icon"},
    {"otf",  "font/opentype"}
};

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
    std::string url = request.getUrl();

    if (url.empty() || url == "/")
	url = "/index.html";

    std::string file = m_documentRoot + "/" + url;

    int fd = open(file.c_str(), O_RDONLY);

    if (fd < 0)
	return nullptr;

    std::unique_ptr<httpserver::HttpResponse> resp =
	std::unique_ptr<httpserver::HttpResponse>(request.createFileResponse(fd));
    setContentType(*resp, url);

    return resp;
}

// =====================================================================================================================
void FileServer::setContentType(httpserver::HttpResponse& response, const std::string& url)
{
    std::string::size_type p = url.rfind('.');

    if (p == std::string::npos)
	return;

    auto it = s_contentTypeMap.find(url.substr(p + 1));

    if (it == s_contentTypeMap.end())
    {
	LOG("file-server: there is no content type mapping for extension: " << url.substr(p + 1));
	return;
    }

    response.addHeader(s_contentType, it->second);
}
