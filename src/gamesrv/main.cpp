#include <cstdio>
#include <cstdlib>

#include "common/Log.hpp"
#include "common/Config.hpp"
#include "common/Socket.hpp"

#include "server/Server.hpp"

using namespace ambition;

int main() {
	Config conf("server.conf");
	
	log("System") % Log::idgaf << "Starting...";	
	Log::getStandardOut()->setMinLevel(Log::error);
	log("ConfigTest") % Log::idgaf << conf.get("test", "abc");

	Server server(false);
	server.start();
}
