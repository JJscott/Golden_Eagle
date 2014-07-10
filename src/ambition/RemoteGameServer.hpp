#ifndef REMOTEGAMESERVER_HPP
#define REMOTEGAMESERVER_HPP

#include "GameServer.hpp"
#include "ambition/Log.hpp"
#include "ambition/ClientSocket.hpp"

namespace ambition {
	class ActionResult {

	};

	class RemoteGameServer : public GameServer {
		ClientSocket csocket;
		bool connection_complete(SocketResult);
	public:
		RemoteGameServer(std::string hostname, uint16_t port);
		int get_game_version() const override {
			
			// TODO
			return 0;
		}
	};
}

#endif