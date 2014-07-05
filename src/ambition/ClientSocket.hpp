#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "ambition/Event.hpp"

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
