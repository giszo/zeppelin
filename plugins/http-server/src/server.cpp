#include "server.h"
#include "request.h"

#include <plugin/pluginmanager.h>
#include <logger.h>

#include <cstring>
#include <sys/stat.h>

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
void Server::start(const Json::Value& config, zeppelin::plugin::PluginManager& pm)
{
    // make sure we have a port configured
    if (!config.isMember("port") || !config["port"].isInt())
    {
	LOG("http-server: port not configured properly!");
	return;
    }

    pm.registerInterface("http-server", this);

    m_daemon = MHD_start_daemon(
	MHD_USE_SELECT_INTERNALLY,
	config["port"].asInt(),
	NULL,
	NULL,
	_requestHandler,
	this,
	MHD_OPTION_NOTIFY_COMPLETED, _requestCompleted, NULL,
	MHD_OPTION_THREAD_POOL_SIZE, 3,
	MHD_OPTION_END);

    if (!m_daemon)
	LOG("http-server: unable to start daemon on port " << config["port"].asInt());
}

// =====================================================================================================================
void Server::stop()
{
    // TODO
}

// =====================================================================================================================
void Server::registerHandler(const std::string& url, const Handler& handler)
{
    size_t i = 0;

    while (i < m_handlers.size() && url.size() < m_handlers[i].m_baseUrl.size())
	++i;

    m_handlers.insert(m_handlers.begin() + i, {url, handler});
}

// =====================================================================================================================
void Server::sendResponse(const httpserver::BufferedHttpResponse& response)
{
    const std::string& data = response.getBuffer();
    MHD_Response* resp = MHD_create_response_from_data(
	     data.size(),
	     (void*)data.c_str(),
	     MHD_NO,
	     MHD_YES);

    // set the headers on the response
    sendHeaders(resp, response.getHeaders());

    MHD_queue_response(static_cast<const MHDHttpRequest&>(response.getRequest()).getConnection(),
		       response.getStatus(),
		       resp);
    MHD_destroy_response(resp);
}

// =====================================================================================================================
void Server::sendResponse(const httpserver::FileHttpResponse& response)
{
    // TODO: error handling
    struct stat st;
    fstat(response.getFd(), &st);

    MHD_Response* resp = MHD_create_response_from_fd(st.st_size, response.getFd());

    // set the headers on the response
    sendHeaders(resp, response.getHeaders());

    MHD_queue_response(static_cast<const MHDHttpRequest&>(response.getRequest()).getConnection(),
		       response.getStatus(),
		       resp);
    MHD_destroy_response(resp);
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
	request = new MHDHttpRequest(connection, method, url);
	*conCls = request;
	return MHD_YES;
    }

    if (*uploadDataSize > 0)
    {
	request->processUploadData(uploadData, *uploadDataSize);
	*uploadDataSize = 0;
	return MHD_YES;
    }

    LOG("http-server: serving request: " << request->getUrl());

    try
    {
	// find a handler for the request
	auto handler = lookupHandler(request->getUrl());

	// execute the handler
	std::unique_ptr<httpserver::HttpResponse> response = handler(*request);

	if (response)
	    response->send(*this);
	else
	    sendNotFound(connection);
    }
    catch (const HandlerNotFoundException&)
    {
	sendNotFound(connection);
    }

    return MHD_YES;
}

// =====================================================================================================================
auto Server::lookupHandler(const std::string& url) const -> const Handler&
{
    for (size_t i = 0; i < m_handlers.size(); ++i)
    {
	const HandlerItem& item = m_handlers[i];

	if (url.size() < item.m_baseUrl.size())
	    continue;

	if (url.compare(0, item.m_baseUrl.size(), item.m_baseUrl) == 0)
	    return item.m_handler;
    }

    throw HandlerNotFoundException();
}

// =====================================================================================================================
void Server::sendHeaders(MHD_Response* response, const std::unordered_map<std::string, std::string>& headers)
{
    for (const auto& it : headers)
	MHD_add_response_header(response, it.first.c_str(), it.second.c_str());
}

// =====================================================================================================================
void Server::sendNotFound(MHD_Connection* connection)
{
    static const char* error = "Not found";

    MHD_Response* resp = MHD_create_response_from_data(strlen(error), (void*)error, MHD_NO, MHD_NO);
    MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, resp);
    MHD_destroy_response(resp);
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
