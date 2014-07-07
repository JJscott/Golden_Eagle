#include "Server.hpp"

namespace ambition {
	Server::Server() {
		isPublic = false;

		// Packet identifiers
		mfactory.add_handler(ambition::packets::packet_id::client_to_server_hello, new ambition::packets::cs_hello_packet_factory);
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