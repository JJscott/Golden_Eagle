#include <iostream>
#include <queue>
#include <thread>
#include <cstdlib>

#include <ambition/GameServer.hpp>
#include <ambition/LocalGameServer.hpp>
#include <ambition/RemoteGameServer.hpp>

bool on_sv_ready(int dummy) {
	std::cout << "Server ready" << std::endl;

	return false;
}

int main(int argc, char** argv) {
	bool use_local = false;

	std::string hostname;
	uint16_t port = 8119;

	std::queue<std::string> cmds;

	for(int i = 0; i < argc; i++)
		cmds.push(argv[i]);

	std::string cmd;
	std::string next;

	while(!cmds.empty()) {
		cmd = cmds.front();
		cmds.pop();
		next = cmds.front();

		if(cmd == "-p") {
			port = atoi(next.c_str());
			cmds.pop();
		} else if(cmd == "--local") {
			use_local = true;
		} else if(!use_local) {
			hostname = cmd;
		}
	}

	ambition::GameServer *sv;
	if(use_local) sv = new ambition::LocalGameServer();
	else sv = new ambition::RemoteGameServer(hostname, port);
	
	sv->ready.attach(on_sv_ready);
}


