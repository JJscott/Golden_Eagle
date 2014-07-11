#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Server.hpp>
#include <ambition/ClientSocket.hpp>
#include <ambition/Concurrent.hpp>

#include <CryptoPP/osrng.h>

using namespace ambition;

// haha globals! :)
int toSend = 0;
std::vector<ClientSocket*> clients;

bool recieved(SocketResult sr) {
	// std::cout << "recv: " << sr.data.read() << std::endl;
	if(sr.success) {
		// send!
		toSend += clients.size();
		for(auto c : clients) {
			if(c != sr.client) {
				std::cout << "sending" << std::endl;
				c->begin_send(sr.data);
			}
		}
	}
	return false;
}

bool sent(SocketResult sr) {
	return false;
}

bool accepted(SocketResult sr) {
	if(sr.success) {
		ClientSocket* nc = sr.client;
		nc->on_recieved.attach(recieved);
		nc->on_sent.attach(sent);
		clients.push_back(nc);
		std::cout << "accepted" << std::endl;
		
		
	}
	return false;
}

int main() {

	CryptoPP::AutoSeededRandomPool rand;
	log("test") << rand.GenerateWord32();

	ListenSocket ls;
	ls.on_accepted.attach(accepted);
	while(true) {
		std::string cmd;
		std::cin >> cmd;
		if(cmd == "quit")
			break;
	}
}
