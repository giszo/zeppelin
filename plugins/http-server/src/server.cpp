#include "server.h"
#include "request.h"

#include <plugin/pluginmanager.h>
#include <logger.h>

#include <cstring>

// =====================================================================================================================
Server::Server()
    : m_daemon(NULL)
{
}

// =====================================================================================================================
Server::~Server()
{
    if (m_daemon)
	MHD_stop_daemon(m_daemon);
}

// =====================================================================================================================
void Server::start(const Json::Value& config, plugin::PluginManager& pm)
{
    // make sure we have a port configured
    if (!config.isMember("port") || !config["port"].isInt())
    {
	LOG("http-server: port not configured properly!");
	return;
    }

    pm.registerInterface("http-server", this);

    m_daemon = MHD_start_daemon(
	MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
	config["port"].asInt(),
	NULL,
	NULL,
	_requestHandler,
	this,
	MHD_OPTION_NOTIFY_COMPLETED, _requestCompleted, NULL,
	MHD_OPTION_END);
}

// =====================================================================================================================
void Server::stop()
{
    // TODO
}

// =====================================================================================================================
void Server::registerHandler(const std::string& url, const Handler& handler)
{
    m_handlers[url] = handler;
}

// =====================================================================================================================
int Server::requestHandler(MHD_Connection* connection,
			   const std::string& url,
			   const std::string& method,
			   const std::string& ver,
			   const char* uploadData,
			   size_t* uploadDataSize,
			   void** conCls)
{
    MHDHttpRequest* request = reinterpret_cast<MHDHttpRequest*>(*conCls);

    if (!request)
    {
	request = new MHDHttpRequest(method, url);
	*conCls = request;
	return MHD_YES;
    }

    if (*uploadDataSize > 0)
    {
	request->processUploadData(uploadData, *uploadDataSize);
	*uploadDataSize = 0;
	return MHD_YES;
    }

    // serve the request
    int status = MHD_HTTP_OK;
    MHD_Response* resp;

    auto it = m_handlers.find(request->getUrl());

    if (it != m_handlers.end())
    {
	std::unique_ptr<httpserver::HttpResponse> response = it->second(*request);
	const std::string& data = response->getData();

	status = response->getStatus();
	resp = MHD_create_response_from_data(
	     data.size(),
	     (void*)data.c_str(),
	     MHD_NO,
	     MHD_YES);

	// set the headers on the response
	const auto& headers = response->getHeaders();

	for (const auto& hit : headers)
	    MHD_add_response_header(resp, hit.first.c_str(), hit.second.c_str());
    }
    else
    {
	static const char* error = "Not found";

	status = MHD_HTTP_NOT_FOUND;
	resp = MHD_create_response_from_data(strlen(error), (void*)error, MHD_NO, MHD_NO);
    }

    int ret = MHD_queue_response(connection, status, resp);
    MHD_destroy_response(resp);

    return ret;
}

// =====================================================================================================================
int Server::_requestHandler(void* cls,
			    MHD_Connection* connection,
			    const char* url,
			    const char* method,
			    const char* version,
			    const char* uploadData,
			    size_t* uploadDataSize,
			    void** conCls)
{
    Server* s = reinterpret_cast<Server*>(cls);
    return s->requestHandler(connection, url, method, version, uploadData, uploadDataSize, conCls);
}

// =====================================================================================================================
void Server::_requestCompleted(void* cls,
			      MHD_Connection* connection,
			      void** conCls,
			      MHD_RequestTerminationCode toe)
{
    MHDHttpRequest* request = reinterpret_cast<MHDHttpRequest*>(*conCls);

    if (!request)
	return;

    // delete the request object
    delete request;
}
