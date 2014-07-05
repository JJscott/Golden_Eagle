#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include <cstdio>
#include <cstdint>

namespace ambition {
	class ListenSocket {
		class ListenSocketImpl;
		ListenSocketImpl *lsock;

	public:

		ListenSocket();
		~ListenSocket();

		void init();
		uint16_t listen_port();
	};
}

#endif