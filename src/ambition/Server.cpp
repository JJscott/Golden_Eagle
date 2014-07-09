#include "Server.hpp"

namespace ambition {
	Server::Server() {
		isPublic = false;
	}

	Server::Server(bool shouldBePublic) {
		isPublic = shouldBePublic;
	}

	void Server::start() {
		log("Server") % 0 << "Starting..";
	}

	uint16_t Server::listen_port() {
		return lsocket->listen_port();
	}

	int Server::get_version() {
		return 0;		
	}
}