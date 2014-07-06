#include "Server.hpp"

namespace ambition {
	Server::Server() {
		isPublic = false;

		// Packet identifiers
		mfactory.add_handler(new ambition::packets::HelloPacket);
	}

	Server::Server(bool shouldBePublic) {
		isPublic = shouldBePublic;
	}

	void Server::start() {
		log("Server") % 0 << "Started!";

		lsocket = new ListenSocket;

	}

	uint16_t Server::listen_port() {
		return lsocket->listen_port();
	}

	int Server::get_version() {
		return 0;		
	}
}