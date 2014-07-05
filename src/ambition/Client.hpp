#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ambition/Log.hpp"

namespace ambition {
	class Client {
	public:
		void target(std::string hostname, uint16_t port);
	};
}

#endif