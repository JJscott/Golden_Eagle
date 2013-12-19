#include "Server.hpp"

#include "common/sg/Scene.hpp"


Server::Server(bool shouldBePublic) {
	isPublic = shouldBePublic;
}

void Server::start() {
	log("Server") % Log::idgaf << "Started!";	

	Scene *s = new Scene;
}