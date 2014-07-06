#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/Log.hpp>
#include <ambition/Config.hpp>
#include <ambition/Server.hpp>
#include <ambition/ClientSocket.hpp>
#include <ambition/Concurrent.hpp>

using namespace ambition;


//class EventTest : public Event<EventTest> {
//public:
//	EventTest() {}
//	~EventTest() {}
//
//	void doTest() { notify(); }
//	void testTest() { log("EventTest") % Log::information << "Testing the test"; }
//};
//
//class EventDelegate : public Delegate<EventTest> {
//public:
//	EventDelegate() {}
//	~EventDelegate() {}
//	void fire(EventTest *et) {
//		log("EventTest") % Log::information << "Fired test event";
//		et->testTest();
//	}
//};

bool connected(SocketResult se) {
	std::cout << "connection complete: status: " << (se.success ? "true" : "false") << std::endl;
	return false;
}

void bad() {
	ClientSocket cs;
	cs.on_connected.attach(connected);
	cs.begin_connect("localhost", 8118, 1000000);
	cs.on_connected.wait();
	return;
}

int main() {
	bad();
	return 0;
	
	ListenSocket socket;
	socket.init();


	// Config conf("server.conf");
	
	log("System") % 0 << "Starting...";
	// log("ConfigTest") % Log::idgaf << conf.get("test", "abc");

	//EventTest et;
	//EventDelegate ed;
	//et.attach(ed);
	//et.notify();

	Server server(false);
	server.start();


	try {
	
	} catch(const char* m) {
		printf("%s\n", m);
	}

	std::cout << "Hit enter to exit" << std::endl;
	std::cin.get();
}
