#ifndef RPC_SERVER_H_INCLUDED
#define RPC_SERVER_H_INCLUDED

#include <library/musiclibrary.h>
#include <player/controller.h>
#include <config/config.h>

#include <jsonrpc/rpc.h>

namespace rpc
{

class Server : public jsonrpc::AbstractServer<Server>
{
    public:
	Server(library::MusicLibrary& library,
	       player::Controller& ctrl,
	       config::RPC& config);

    private:
	void libraryScan(const Json::Value& request, Json::Value& response);

	// library - artists
	void libraryGetArtists(const Json::Value& request, Json::Value& response);

	// library - albums
	void libraryGetAlbums(const Json::Value& request, Json::Value& response);
	void libraryGetAlbumsByArtist(const Json::Value& request, Json::Value& response);

	// library - files
	void libraryGetFilesOfAlbum(const Json::Value& request, Json::Value& response);

	// player - queue
	void playerQueueFile(const Json::Value& request, Json::Value& response);
	void playerQueueAlbum(const Json::Value& request, Json::Value& response);
	void playerQueueGet(const Json::Value& request, Json::Value& response);

	void playerStatus(const Json::Value& request, Json::Value& response);

	void playerPlay(const Json::Value& request, Json::Value& response);
	void playerPause(const Json::Value& request, Json::Value& response);
	void playerStop(const Json::Value& request, Json::Value& response);
	void playerPrev(const Json::Value& request, Json::Value& response);
	void playerNext(const Json::Value& request, Json::Value& response);
	void playerGoto(const Json::Value& request, Json::Value& response);

	void playerSetVolume(const Json::Value& request, Json::Value& response);
	void playerIncVolume(const Json::Value& request, Json::Value& response);
	void playerDecVolume(const Json::Value& request, Json::Value& response);

    private:
	library::MusicLibrary& m_library;
	player::Controller& m_ctrl;
};

}

#endif
