#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <ambition/Concurrent.hpp>
#include <ambition/ByteBuffer.hpp>

// KNOWN ISSUES:
// Thread may try to grab FDSET whilst it's being set by calling thread, needs mutex
// Dtor should call close()

namespace ambition {
	class ClientSocket;
	struct SocketResult {
		bool success;
		int n_bytes;
		ByteBuffer* data;
		ClientSocket* client;
	};

	class ClientSocket {	
		class ClientSocketImpl;
		ClientSocketImpl* cs_;
	public:
		ClientSocket();
		ClientSocket(int ext);
		~ClientSocket();

		Event<SocketResult> on_connected;
		Event<SocketResult> on_sent;
		Event<SocketResult> on_recieved;
		Event<SocketResult> on_closed;

		bool connected();
		void begin_connect(std::string host, uint16_t port, int usec);
		void begin_send(const ByteBuffer &);
	};

}

#endif
