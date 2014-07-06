#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Server.hpp>
#include <ambition/ClientSocket.hpp>
#include <ambition/Concurrent.hpp>

using namespace ambition;

bool connected(SocketResult se) {
	std::cout << "Connected!" << std::endl;
	return false;
}

bool recieved(SocketResult se) {
	std::cout << "Recieved(" << se.n_bytes << "): " << se.data->get_string() << std::endl;
	return false;
}

int main() {
	ClientSocket cs;
	cs.on_connected.attach(connected);
	cs.on_recieved.attach(recieved);
	cs.begin_connect("localhost", 8118, 1000000);
	cs.on_connected.wait();
	if(!cs.connected()) {
		std::cout << "Unable to connect" << std::endl;
		return 0;
	}

	ByteBuffer bb1;
	bb1.add_string("Hello");
	cs.begin_send(bb1);

	ByteBuffer bb2;
	bb2.add_string(", World!");
	cs.begin_send(bb2);

	cs.on_closed.wait();
	return 0;
}
