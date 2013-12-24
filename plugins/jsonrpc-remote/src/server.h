#ifndef JSONRPCREMOTE_SERVER_H_INCLUDED
#define JSONRPCREMOTE_SERVER_H_INCLUDED

#include <library/musiclibrary.h>
#include <player/controller.h>
#include <config/config.h>
#include <plugin/plugin.h>

#include <jsonrpc/rpc.h>

class Server : public plugin::Plugin,
	       public jsonrpc::AbstractServer<Server>
{
    public:
	Server(int port,
	       const std::shared_ptr<library::MusicLibrary>& library,
	       const std::shared_ptr<player::Controller>& ctrl);

	std::string getName() const override
	{ return "jsonrpc-remote"; }

	void start() override;
	void stop() override;

    private:
	void libraryScan(const Json::Value& request, Json::Value& response);

	// library - artists
	void libraryGetArtists(const Json::Value& request, Json::Value& response);

	// library - albums
	void libraryGetAlbums(const Json::Value& request, Json::Value& response);
	void libraryGetAlbumsByArtist(const Json::Value& request, Json::Value& response);

	// library - files
	void libraryGetFilesOfArtist(const Json::Value& request, Json::Value& response);
	void libraryGetFilesOfAlbum(const Json::Value& request, Json::Value& response);

	// player - queue
	void playerQueueFile(const Json::Value& request, Json::Value& response);
	void playerQueueAlbum(const Json::Value& request, Json::Value& response);
	void playerQueueGet(const Json::Value& request, Json::Value& response);
	void playerQueueRemove(const Json::Value& request, Json::Value& response);

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
	std::unique_ptr<jsonrpc::AbstractServer<Server>> m_server;

	std::shared_ptr<library::MusicLibrary> m_library;
	std::shared_ptr<player::Controller> m_ctrl;
};

#endif
