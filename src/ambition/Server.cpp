#include "Server.hpp"

#include "ambition/Scene.hpp"


Server::Server(bool shouldBePublic) {
	isPublic = shouldBePublic;
}

void Server::start() {
	log("Server") % 0 << "Started!";	

	Scene *s = new Scene;
}