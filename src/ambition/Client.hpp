#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ambition/Log.hpp"

namespace ambition {
	class Client {
	public:
		void target(std::string hostname, uint16_t port);
		void block_until_connected(int ms_timeout);
		bool connected();
		int get_server_version();
		int get_version();
	};
}

#endif