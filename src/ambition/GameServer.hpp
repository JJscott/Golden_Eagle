#ifndef SERVERVIEW_HEADER
#define SERVERVIEW_HEADER

#include <ambition/Concurrent.hpp>

namespace ambition {
	class GameServer {
	public:
		Event<int> ready;
	};
}

#endif