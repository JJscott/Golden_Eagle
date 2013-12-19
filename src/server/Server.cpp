#include "Server.hpp"

#include "common/Scene.hpp"


Server::Server(bool shouldBePublic) {
	isPublic = shouldBePublic;
}

void Server::start() {
	log("Server") % Log::idgaf << "Started!";	

	Scene *s = new Scene;
}