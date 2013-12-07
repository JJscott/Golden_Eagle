#include "Server.hpp"



Server::Server(bool shouldBePublic) {
	isPublic = shouldBePublic;
}

void Server::start() {
	log("Server") % Log::idgaf << "Started!";	
}