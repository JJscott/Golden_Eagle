#include "Server.hpp"

#include "ambition/Scene.hpp"


Server::Server(bool shouldBePublic) {
	isPublic = shouldBePublic;
}

void Server::start() {
	log("Server") % Log::idgaf << "Started!";	

	Scene *s = new Scene;
}