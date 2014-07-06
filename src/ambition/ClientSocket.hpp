#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <ambition/Concurrent.hpp>

// KNOWN ISSUES:
// Thread may try to grab FDSET whilst it's being set by calling thread, needs mutex
// Needs send, recieve, close
// Dtor should call close()

namespace ambition {
	struct SocketResult {
		bool success;
	};

	class ClientSocket {	
		class ClientSocketImpl;
		ClientSocketImpl* cs_;
	public:
		ClientSocket();
		~ClientSocket();
		Event<SocketResult> on_connected;

		void begin_connect(std::string host, uint16_t port, int usec);
	};

}

#endif
