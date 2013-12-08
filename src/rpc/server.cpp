#include "server.h"

#include <iostream>

using rpc::Server;

// =====================================================================================================================
Server::Server(library::MusicLibrary& library, player::Controller& ctrl)
    : AbstractServer<Server>(new jsonrpc::HttpServer(8080, false)),
      m_library(library),
      m_ctrl(ctrl)
{
    // library
    bindAndAddMethod(new jsonrpc::Procedure("library_scan_directory",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "path",
					    jsonrpc::JSON_STRING,
					    NULL),
		     &Server::libraryScanDirectory);
    bindAndAddMethod(new jsonrpc::Procedure("library_list_files",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::libraryListFiles);

    // player queue
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_file",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::playerQueueFile);
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_get",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::playerQueueGet);

    // player control
    bindAndAddMethod(new jsonrpc::Procedure("player_play",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerPlay);
    bindAndAddMethod(new jsonrpc::Procedure("player_stop",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerStop);
}

// =====================================================================================================================
void Server::libraryScanDirectory(const Json::Value& request, Json::Value& response)
{
    m_library.scanDirectory(request["path"].asString());
}

// =====================================================================================================================
void Server::libraryListFiles(const Json::Value& request, Json::Value& response)
{
    std::vector<library::File> files = m_library.getFileList();
    std::cout << "files: " << files.size() << std::endl;

    response = Json::Value(Json::arrayValue);

    for (const auto& f : files)
    {
	Json::Value file(Json::objectValue);
	file["id"] = f.m_id;
	file["path"] = f.m_path;
	file["name"] = f.m_name;
	file["length"] = f.m_length;

	response.append(file);
    }
}

// =====================================================================================================================
void Server::playerQueueFile(const Json::Value& request, Json::Value& response)
{
    std::shared_ptr<library::File> file;

    try
    {
	file = m_library.getFile(request["id"].asInt());
    }
    catch (const library::FileNotFoundException& e)
    {
	std::cerr << "File not found with ID: " << request["id"] << std::endl;
	return;
    }

    std::cout << "Queueing file: " << file->m_path << "/" << file->m_name << std::endl;

    m_ctrl.queue(file);
}

// =====================================================================================================================
void Server::playerQueueGet(const Json::Value& request, Json::Value& response)
{
    auto queue = m_ctrl.getQueue();

    response = Json::Value(Json::arrayValue);

    for (const auto& f : queue)
    {
	Json::Value file(Json::objectValue);
	file["id"] = f->m_id;
	file["path"] = f->m_path;
	file["name"] = f->m_name;
	file["length"] = f->m_length;

	response.append(file);
    }
}

// =====================================================================================================================
void Server::playerPlay(const Json::Value& request, Json::Value& response)
{
    m_ctrl.play();
}

// =====================================================================================================================
void Server::playerStop(const Json::Value& request, Json::Value& response)
{
    m_ctrl.stop();
}
