#ifndef JSONRPCREMOTE_SERVER_H_INCLUDED
#define JSONRPCREMOTE_SERVER_H_INCLUDED

#include <plugins/http-server/httpserver.h>

#include <library/musiclibrary.h>
#include <player/controller.h>
#include <config/config.h>
#include <plugin/plugin.h>

#include <jsoncpp/json/value.h>

#include <unordered_map>

class Server : public plugin::Plugin
{
    public:
	Server(const std::shared_ptr<library::MusicLibrary>& library,
	       const std::shared_ptr<player::Controller>& ctrl);

	std::string getName() const override
	{ return "jsonrpc-remote"; }

	void start(const Json::Value& config, plugin::PluginManager& pm) override;
	void stop() override;

    private:
	std::unique_ptr<httpserver::HttpResponse> processRequest(const httpserver::HttpRequest& request);

	void libraryScan(const Json::Value& request, Json::Value& response);

	// library - artists
	void libraryGetArtists(const Json::Value& request, Json::Value& response);

	// library - albums
	void libraryGetAlbums(const Json::Value& request, Json::Value& response);
	void libraryGetAlbumsByArtist(const Json::Value& request, Json::Value& response);

	// library - files
	void libraryGetFilesOfArtist(const Json::Value& request, Json::Value& response);
	void libraryGetFilesOfAlbum(const Json::Value& request, Json::Value& response);

	void libraryUpdateMetadata(const Json::Value& request, Json::Value& response);

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

	void playerGetVolume(const Json::Value& request, Json::Value& response);
	void playerSetVolume(const Json::Value& request, Json::Value& response);
	void playerIncVolume(const Json::Value& request, Json::Value& response);
	void playerDecVolume(const Json::Value& request, Json::Value& response);

    private:
	std::shared_ptr<library::MusicLibrary> m_library;
	std::shared_ptr<player::Controller> m_ctrl;

	typedef std::function<void(const Json::Value&, Json::Value&)> RpcMethod;

	std::unordered_map<std::string, RpcMethod> m_rpcMethods;
};

#endif
