#ifndef RPC_SERVER_H_INCLUDED
#define RPC_SERVER_H_INCLUDED

#include <library/musiclibrary.h>
#include <player/controller.h>

#include <jsonrpc/rpc.h>

namespace rpc
{

class Server : public jsonrpc::AbstractServer<Server>
{
    public:
	Server(library::MusicLibrary& library, player::Controller& ctrl);

    private:
	void libraryScanDirectory(const Json::Value& request, Json::Value& response);
	void libraryListFiles(const Json::Value& request, Json::Value& response);

	void playerQueueFile(const Json::Value& request, Json::Value& response);
	void playerQueueGet(const Json::Value& request, Json::Value& response);

	void playerPlay(const Json::Value& request, Json::Value& response);
	void playerStop(const Json::Value& request, Json::Value& response);

    private:
	library::MusicLibrary& m_library;
	player::Controller& m_ctrl;
};

}

#endif
