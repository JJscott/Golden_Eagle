#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "ambition/Log.hpp"
#include "ambition/Config.hpp"
#include "ambition/Socket.hpp"
#include "ambition/Server.hpp"
#include "ambition/Event.hpp"
#include "ambition/Scene.hpp"
#include "ambition/TreeEntity.hpp"

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

int main() {
	// Config conf("server.conf");
	Scene *scene = new Scene;
	scene->addEntity(new TreeEntity);
	
	log("System") % 0 << "Starting...";
	// log("ConfigTest") % Log::idgaf << conf.get("test", "abc");

	//EventTest et;
	//EventDelegate ed;
	//et.attach(ed);
	//et.notify();

	Server server(false);
	server.start();


	try {
	ListenSocket socket;
	socket.init();
	} catch(const char* m) {
		printf("%s\n", m);
	}

	std::cout << "Hit enter to exit" << std::endl;
	std::cin.get();
}
