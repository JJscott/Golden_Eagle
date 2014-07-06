#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Server.hpp>
#include <ambition/ClientSocket.hpp>
#include <ambition/Concurrent.hpp>

using namespace ambition;


// haha globals! :)
int toSend = 0;
std::vector<ClientSocket*> clients;

bool recieved(SocketResult sr) {
	if(sr.success) {
		// send!
		toSend += clients.size();
		for(auto c : clients) {
			std::cout << "sending" << std::endl;
			c->begin_send(*sr.data);
		}
	}
	return false;
}

bool sent(SocketResult sr) {
	return false;
}

bool accepted(SocketResult sr) {
	if(sr.success) {
		ClientSocket* nc = sr.new_client;
		clients.push_back(nc);
		nc->on_recieved.attach(recieved);
		nc->on_sent.attach(sent);
	}
	return false;
}

int main() {
	ListenSocket ls;
	ls.on_accepted.attach(accepted);
	while(true) {
		std::string cmd;
		std::cin >> cmd;
		if(cmd == "quit")
			break;
	}
}
