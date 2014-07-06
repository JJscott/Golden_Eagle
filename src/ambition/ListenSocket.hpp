#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include "Concurrent.hpp"
#include "ClientSocket.hpp"

#include <cstdio>
#include <cstdint>

namespace ambition {
	class ListenSocket {
		class ListenSocketImpl;
		ListenSocketImpl *lsock;

	public:
		Event<SocketResult> on_accepted;
		ListenSocket();
		~ListenSocket();

		void init();
		uint16_t listen_port();
	};
}

#endif