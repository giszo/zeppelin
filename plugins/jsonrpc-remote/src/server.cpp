#include "server.h"

#include <iostream>

// =====================================================================================================================
Server::Server(int port,
	       const std::shared_ptr<library::MusicLibrary>& library,
	       const std::shared_ptr<player::Controller>& ctrl)
    : AbstractServer<Server>(new jsonrpc::HttpServer(port, false, "", 3 /* use 3 worker threads */)),
      m_library(library),
      m_ctrl(ctrl)
{
    // library
    bindAndAddMethod(new jsonrpc::Procedure("library_scan",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
			       &Server::libraryScan);

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

    // library - files
    bindAndAddMethod(new jsonrpc::Procedure("library_get_files_of_artist",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    "artist_id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::libraryGetFilesOfArtist);
    bindAndAddMethod(new jsonrpc::Procedure("library_get_files_of_album",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_ARRAY,
					    "album_id",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::libraryGetFilesOfAlbum);

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
    bindAndAddMethod(new jsonrpc::Procedure("player_queue_remove",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "index",
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::playerQueueRemove);

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
    bindAndAddMethod(new jsonrpc::Procedure("player_goto",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "index",
					    jsonrpc::JSON_ARRAY,
					    NULL),
		     &Server::playerGoto);
    bindAndAddMethod(new jsonrpc::Procedure("player_set_volume",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    "level",
					    jsonrpc::JSON_INTEGER,
					    NULL),
		     &Server::playerSetVolume);
    bindAndAddMethod(new jsonrpc::Procedure("player_inc_volume",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerIncVolume);
    bindAndAddMethod(new jsonrpc::Procedure("player_dec_volume",
					    jsonrpc::PARAMS_BY_NAME,
					    jsonrpc::JSON_NULL,
					    NULL),
		     &Server::playerDecVolume);
}

// =====================================================================================================================
void Server::start()
{
    if (!StartListening())
	std::cout << "jsonrpc-remote: unable to start listening!" << std::endl;
}

// =====================================================================================================================
void Server::stop()
{
}

// =====================================================================================================================
void Server::libraryScan(const Json::Value& request, Json::Value& response)
{
    m_library->scan();
}

// =====================================================================================================================
void Server::libraryGetArtists(const Json::Value& request, Json::Value& response)
{
    auto artists = m_library->getStorage().getArtists();

    response = Json::Value(Json::arrayValue);
    response.resize(artists.size());

    for (size_t i = 0; i < artists.size(); ++i)
    {
	const auto& a = artists[i];

	Json::Value artist(Json::objectValue);
	artist["id"] = a->m_id;
	artist["name"] = a->m_name;
	artist["albums"] = a->m_albums;
	artist["songs"] = a->m_songs;

	response[i].swap(artist);
    }
}

// =====================================================================================================================
void Server::libraryGetAlbums(const Json::Value& request, Json::Value& response)
{
    auto albums = m_library->getStorage().getAlbums();

    response = Json::Value(Json::arrayValue);
    response.resize(albums.size());

    for (size_t i = 0; i < albums.size(); ++i)
    {
	const auto& a = albums[i];

	Json::Value album(Json::objectValue);
	album["id"] = a->m_id;
	album["name"] = a->m_name;
	album["artist"] = a->m_artist;
	album["songs"] = a->m_songs;
	album["length"] = a->m_length;

	response[i].swap(album);
    }
}

// =====================================================================================================================
void Server::libraryGetAlbumsByArtist(const Json::Value& request, Json::Value& response)
{
    auto albums = m_library->getStorage().getAlbumsByArtist(request["artist_id"].asInt());

    response = Json::Value(Json::arrayValue);
    response.resize(albums.size());

    for (size_t i = 0; i < albums.size(); ++i)
    {
	const auto& a = albums[i];

	Json::Value album(Json::objectValue);
	album["id"] = a->m_id;
	album["name"] = a->m_name;
	album["songs"] = a->m_songs;
	album["length"] = a->m_length;

	response[i].swap(album);
    }
}

// =====================================================================================================================
void Server::libraryGetFilesOfArtist(const Json::Value& request, Json::Value& response)
{
    auto files = m_library->getStorage().getFilesOfArtist(request["artist_id"].asInt());

    response = Json::Value(Json::arrayValue);
    response.resize(files.size());

    for (size_t i = 0; i < files.size(); ++i)
    {
	const auto& f = files[i];

	Json::Value file(Json::objectValue);
	file["id"] = f->m_id;
	file["path"] = f->m_path;
	file["name"] = f->m_name;
	file["length"] = f->m_length;
	file["title"] = f->m_title;
	file["year"] = f->m_year;

	response[i].swap(file);
    }
}

// =====================================================================================================================
void Server::libraryGetFilesOfAlbum(const Json::Value& request, Json::Value& response)
{
    auto files = m_library->getStorage().getFilesOfAlbum(request["album_id"].asInt());

    response = Json::Value(Json::arrayValue);
    response.resize(files.size());

    for (size_t i = 0; i < files.size(); ++i)
    {
	const auto& f = files[i];

	Json::Value file(Json::objectValue);
	file["id"] = f->m_id;
	file["path"] = f->m_path;
	file["name"] = f->m_name;
	file["length"] = f->m_length;
	file["title"] = f->m_title;
	file["year"] = f->m_year;

	response[i].swap(file);
    }
}

// =====================================================================================================================
void Server::playerQueueFile(const Json::Value& request, Json::Value& response)
{
    std::shared_ptr<library::File> file;

    try
    {
	file = m_library->getStorage().getFile(request["id"].asInt());
    }
    catch (const library::FileNotFoundException& e)
    {
	std::cerr << "File not found with ID: " << request["id"] << std::endl;
	return;
    }

    std::cout << "Queueing file: " << file->m_path << "/" << file->m_name << std::endl;

    m_ctrl->queue(file);
}

// =====================================================================================================================
void Server::playerQueueAlbum(const Json::Value& request, Json::Value& response)
{
    int albumId = request["id"].asInt();

    std::cout << "Queueing album: " << albumId << std::endl;

    // TODO: handle not found exception here!
    auto album = m_library->getStorage().getAlbum(albumId);
    auto files = m_library->getStorage().getFilesOfAlbum(albumId);

    m_ctrl->queue(album, files);
}

// =====================================================================================================================
static inline void serializeQueueItem(Json::Value& parent, const std::shared_ptr<player::QueueItem>& item)
{
    Json::Value qi(Json::objectValue);
    qi["type"] = item->type();

    switch (item->type())
    {
	case player::QueueItem::PLAYLIST :
	    break;

	case player::QueueItem::ALBUM :
	{
	    const player::Album& ai = static_cast<const player::Album&>(*item);
	    const library::Album& album = ai.album();

	    qi["id"] = album.m_id;
	    qi["name"] = album.m_name;
	    qi["files"] = Json::Value(Json::arrayValue);

	    for (const auto& i : ai.items())
		serializeQueueItem(qi["files"], i);

	    break;
	}

	case player::QueueItem::FILE :
	{
	    auto file = item->file();

	    qi["id"] = file->m_id;
	    qi["path"] = file->m_path;
	    qi["name"] = file->m_name;
	    qi["title"] = file->m_title;
	    qi["length"] = file->m_length;

	    break;
	}
    }

    parent.append(qi);
}

// =====================================================================================================================
void Server::playerQueueGet(const Json::Value& request, Json::Value& response)
{
    auto queue = m_ctrl->getQueue();

    response = Json::Value(Json::arrayValue);

    for (const auto& item : queue->items())
	serializeQueueItem(response, item);
}

// =====================================================================================================================
void Server::playerQueueRemove(const Json::Value& request, Json::Value& response)
{
    Json::Value index = request["index"];

    std::vector<int> i;

    for (Json::UInt j = 0; j < index.size(); ++j)
	i.push_back(index[j].asInt());

    m_ctrl->remove(i);
}

// =====================================================================================================================
void Server::playerStatus(const Json::Value& request, Json::Value& response)
{
    player::Status s = m_ctrl->getStatus();

    response = Json::Value(Json::objectValue);

    response["current"] = s.m_file ? Json::Value(s.m_file->m_id) : Json::Value(Json::nullValue);
    response["state"] = static_cast<int>(s.m_state);
    response["position"] = s.m_position;
    response["volume"] = s.m_volume;
}

// =====================================================================================================================
void Server::playerPlay(const Json::Value& request, Json::Value& response)
{
    m_ctrl->play();
}

// =====================================================================================================================
void Server::playerPause(const Json::Value& request, Json::Value& response)
{
    m_ctrl->pause();
}

// =====================================================================================================================
void Server::playerStop(const Json::Value& request, Json::Value& response)
{
    m_ctrl->stop();
}

// =====================================================================================================================
void Server::playerPrev(const Json::Value& request, Json::Value& response)
{
    m_ctrl->prev();
}

// =====================================================================================================================
void Server::playerNext(const Json::Value& request, Json::Value& response)
{
    m_ctrl->next();
}

// =====================================================================================================================
void Server::playerGoto(const Json::Value& request, Json::Value& response)
{
    Json::Value index = request["index"];

    std::vector<int> i;

    for (Json::UInt j = 0; j < index.size(); ++j)
	i.push_back(index[j].asInt());

    m_ctrl->goTo(i);
}

// =====================================================================================================================
void Server::playerSetVolume(const Json::Value& request, Json::Value& response)
{
    m_ctrl->setVolume(request["level"].asInt());
}

// =====================================================================================================================
void Server::playerIncVolume(const Json::Value& request, Json::Value& response)
{
    m_ctrl->incVolume();
}

// =====================================================================================================================
void Server::playerDecVolume(const Json::Value& request, Json::Value& response)
{
    m_ctrl->decVolume();
}
