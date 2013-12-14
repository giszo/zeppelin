#include "server.h"

#include <iostream>

using rpc::Server;

// =====================================================================================================================
Server::Server(library::MusicLibrary& library,
	       player::Controller& ctrl,
	       config::RPC& config)
    : AbstractServer<Server>(new jsonrpc::HttpServer(config.m_port, false, "", 3 /* use 3 worker threads */)),
      m_library(library),
      m_ctrl(ctrl)
{
    // library
    bindAndAddMethod(new jsonrpc::Procedure("library_scan",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::libraryScan);
    bindAndAddMethod(new jsonrpc::Procedure("library_list_files",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::libraryListFiles);

    // library - artists
    bindAndAddMethod(new jsonrpc::Procedure("library_get_artists",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::libraryGetArtists);

    // library - albums
    bindAndAddMethod(new jsonrpc::Procedure("library_get_albums",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::libraryGetAlbums);
    bindAndAddMethod(new jsonrpc::Procedure("library_get_albums_by_artist",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    "artist_id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::libraryGetAlbumsByArtist);

    // player queue
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_file",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::playerQueueFile);
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_album",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::playerQueueAlbum);
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_get",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::playerQueueGet);

    // player status
    bindAndAddMethod(new jsonrpc::Procedure("player_status",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_OBJECT,
					    NULL),
		     &Server::playerStatus);

    // player control
    bindAndAddMethod(new jsonrpc::Procedure("player_play",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerPlay);
    bindAndAddMethod(new jsonrpc::Procedure("player_pause",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerPause);
    bindAndAddMethod(new jsonrpc::Procedure("player_stop",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerStop);
    bindAndAddMethod(new jsonrpc::Procedure("player_prev",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerPrev);
    bindAndAddMethod(new jsonrpc::Procedure("player_next",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerNext);
}

// =====================================================================================================================
void Server::libraryScan(const Json::Value& request, Json::Value& response)
{
    m_library.scan();
}

// =====================================================================================================================
void Server::libraryListFiles(const Json::Value& request, Json::Value& response)
{
    std::vector<std::shared_ptr<library::File>> files = m_library.getStorage().getFiles();
    std::cout << "files: " << files.size() << std::endl;

    response = Json::Value(Json::arrayValue);

    for (const auto& f : files)
    {
	Json::Value file(Json::objectValue);
	file["id"] = f->m_id;
	file["path"] = f->m_path;
	file["name"] = f->m_name;
	file["length"] = f->m_length;
	file["artist"] = f->m_artist;
	file["album"] = f->m_album;
	file["title"] = f->m_title;
	file["year"] = f->m_year;

	response.append(file);
    }
}

// =====================================================================================================================
void Server::libraryGetArtists(const Json::Value& request, Json::Value& response)
{
    auto artists = m_library.getStorage().getArtists();

    response = Json::Value(Json::arrayValue);

    for (const auto& a : artists)
    {
	Json::Value artist(Json::objectValue);
	artist["id"] = a->m_id;
	artist["name"] = a->m_name;

	response.append(artist);
    }
}

// =====================================================================================================================
void Server::libraryGetAlbums(const Json::Value& request, Json::Value& response)
{
    auto albums = m_library.getStorage().getAlbums();

    response = Json::Value(Json::arrayValue);

    for (const auto& a : albums)
    {
	Json::Value album(Json::objectValue);
	album["id"] = a->m_id;
	album["name"] = a->m_name;
	album["artist"] = a->m_artist;

	response.append(album);
    }
}

// =====================================================================================================================
void Server::libraryGetAlbumsByArtist(const Json::Value& request, Json::Value& response)
{
    auto albums = m_library.getStorage().getAlbumsByArtist(request["artist_id"].asInt());

    response = Json::Value(Json::arrayValue);

    for (const auto& a : albums)
    {
	Json::Value album(Json::objectValue);
	album["id"] = a->m_id;
	album["name"] = a->m_name;

	response.append(album);
    }
}

// =====================================================================================================================
void Server::playerQueueFile(const Json::Value& request, Json::Value& response)
{
    std::shared_ptr<library::File> file;

    try
    {
	file = m_library.getStorage().getFile(request["id"].asInt());
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
void Server::playerQueueAlbum(const Json::Value& request, Json::Value& response)
{
    std::cout << "Queueing album: " << request["id"] << std::endl;

    auto files = m_library.getStorage().getFilesOfAlbum(request["id"].asInt());

    for (const auto& file : files)
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
void Server::playerStatus(const Json::Value& request, Json::Value& response)
{
    player::Status s = m_ctrl.getStatus();

    response = Json::Value(Json::objectValue);

    response["current"] = s.m_file ? Json::Value(s.m_file->m_id) : Json::Value(Json::nullValue);
    response["state"] = static_cast<int>(s.m_state);
    response["position"] = s.m_position;
}

// =====================================================================================================================
void Server::playerPlay(const Json::Value& request, Json::Value& response)
{
    m_ctrl.play();
}

// =====================================================================================================================
void Server::playerPause(const Json::Value& request, Json::Value& response)
{
    m_ctrl.pause();
}

// =====================================================================================================================
void Server::playerStop(const Json::Value& request, Json::Value& response)
{
    m_ctrl.stop();
}

// =====================================================================================================================
void Server::playerPrev(const Json::Value& request, Json::Value& response)
{
    m_ctrl.prev();
}

// =====================================================================================================================
void Server::playerNext(const Json::Value& request, Json::Value& response)
{
    m_ctrl.next();
}
