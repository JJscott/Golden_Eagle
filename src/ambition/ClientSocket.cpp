#include <thread>
#include <cstring>

#include "ClientSocket.hpp"

#ifndef _WIN32
	using SOCKET = int;
	#define INVALID_SOCKET -1
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <errno.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <netdb.h>
	#define FD_CLR_F FD_CLR
	#define FD_SET_F FD_SET

#else
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define FD_CLR_F(fd, set) u_int __i; \
	for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
	if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
		while (__i < ((fd_set FAR *)(set))->fd_count - 1) {	\
			((fd_set FAR *)(set))->fd_array[__i] = \
			((fd_set FAR *)(set))->fd_array[__i + 1]; \
			__i++; \
		} \
		((fd_set FAR *)(set))->fd_count--; \
		break; \
	} \
	}

	#define FD_SET_F(fd, set) if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
		((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++] = (fd);

	
	typedef int socklen_t;
#endif

#define FD_ZERO_F FD_ZERO
#define FD_ISSET_F FD_ISSET

namespace ambition {

	class ClientSocket::ClientSocketImpl {
		fd_set wfdset;
		fd_set rfdset;
		SOCKET client_socket;
		timeval tv;
		std::thread* worker;
		ClientSocket* outer;
		bool connected = false;
		std::vector<char> to_send;
	public:
		ClientSocketImpl(ClientSocket*);
		ClientSocketImpl(ClientSocket*, int);
		static void work_thread(ClientSocketImpl* target);
		bool connected_();
		void begin_connect(std::string, uint16_t, int);
		void begin_send(const byte_buffer &);
	};

	ClientSocket::ClientSocketImpl::ClientSocketImpl(ClientSocket *o) : outer(o) { 
		#ifdef _WIN32
			WSAData data;
			WSAStartup(MAKEWORD(1, 1), &data);
		#endif

		client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(client_socket == INVALID_SOCKET) {
			network_error ne(error::neterr_socket_create_failure, "Unable to create socket");
			ne.error_no = errno;
			ne.error_message = strerror(errno);
			throw ne;
		}

		#ifdef _WIN32
		u_long iMode=1;
		ioctlsocket(client_socket, FIONBIO, &iMode);
		#else
		fcntl(client_socket, F_SETFL, O_NONBLOCK);
		#endif

		if(client_socket == INVALID_SOCKET) {
			network_error ne(error::neterr_socket_set_blocking_failure, "Unable to set socket blocking mode");
			ne.error_no = errno;
			ne.error_message = strerror(errno);
			throw ne;
		}

		FD_ZERO_F(&rfdset);
		FD_ZERO_F(&wfdset);
		FD_SET_F(client_socket, &wfdset);
	}

	ClientSocket::ClientSocketImpl::ClientSocketImpl(ClientSocket *o, int ext) : client_socket(ext), outer(o) {
		connected = true;
	}

	void ClientSocket::ClientSocketImpl::work_thread(ClientSocketImpl* target) {
		timeval tv;
		tv.tv_sec = 5;
		int rv;
		byte_t buffer[2048];
		while(true) {
			rv = select(target->client_socket+1, &(target->rfdset), &(target->wfdset), NULL, NULL);

			if(rv == INVALID_SOCKET) {
				network_error ne(error::neterr_select_failure, "General select() error");
				ne.error_no = errno;
				ne.error_message = strerror(errno);
				throw ne;
			}

			if (rv >= 1 && !target->connected) {
				char so_error;
				socklen_t slen = sizeof(so_error);
				getsockopt(target->client_socket, SOL_SOCKET, SO_ERROR, &so_error, &slen);

				SocketResult sr;
				sr.success = (so_error == 0);

				if(sr.success) {
					target->connected = sr.success;

					FD_CLR_F(target->client_socket, &target->wfdset);
					FD_SET_F(target->client_socket, &target->rfdset);

					
				}
				target->outer->on_connected.notify(sr);				
				if(!sr.success) return;
			} else if(rv >= 1 && target->connected) {
				if(FD_ISSET_F(target->client_socket, &(target->rfdset))) {
					FD_CLR_F(target->client_socket, &(target->rfdset));
					int rx = recv(target->client_socket, buffer, 2048, 0);
					FD_SET_F(target->client_socket, &(target->rfdset));
					
					if(rx == 0) {
						// remote gone away
						target->connected = false;
						throw network_error(error::neterr_lost_connection, "The remote host has gone away");
					}

					if(rx == INVALID_SOCKET) {
						std::cout << "OHSHIT" << std::endl;
					}

					SocketResult sr;
					sr.success = true;
					sr.n_bytes = rx;

					memset(buffer, 0, 2048);

					sr.data = byte_buffer(buffer, rx);
					target->outer->on_recieved.notify(sr);
					
					
				}
				if(FD_ISSET_F(target->client_socket, &(target->wfdset))) {
					FD_CLR_F(target->client_socket, &target->wfdset);
					int rv = send(target->client_socket, &(target->to_send[0]), target->to_send.size(), 0);
					if(rv == INVALID_SOCKET) {
						network_error ne(error::neterr_send_failure, "Unable to send");
						ne.error_no = errno;
						ne.error_message = strerror(errno);
						throw ne;
					}
				}
			}
		}
	}

	
	bool ClientSocket::ClientSocketImpl::connected_() { return connected; }

	void ClientSocket::ClientSocketImpl::begin_connect(std::string hostname, uint16_t pt, int us_timeout) {
		// let's a get a new thread, to deal with the socket polling
		if(connected) throw network_error(error::neterr_already_connected, "Socket already in connected state");
	    tv.tv_usec = us_timeout;

		int rv;

		addrinfo hints;
		addrinfo *rp;

		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		
		int n_zeros =0 ;
		int pt_clone = pt;
		while(pt_clone > 10) { pt_clone /= 10; n_zeros++; }
		char* pt_str = new char[n_zeros];
		sprintf(pt_str,"%d", pt);

		rv = getaddrinfo(hostname.c_str(), pt_str, &hints, &rp);
		if(rv != 0) {
			network_error ne(error::neterr_resolve_failure, "Failed to get address for hostname");
			ne.error_no = errno;
			ne.error_message = strerror(errno);
			throw ne;
		}

		rv = connect(client_socket, rp->ai_addr, rp->ai_addrlen);
		if(rv != INVALID_SOCKET) {
			network_error ne(error::neterr_connect_failure, "Unable to begin connect request");
			ne.error_no = errno;
			ne.error_message = strerror(errno);
			throw ne;
		}

		worker = new std::thread(work_thread, this);
	}

	void ClientSocket::ClientSocketImpl::begin_send(const byte_buffer &bb) {
		if(!connected) {
			throw network_error(error::neterr_not_connected, "Socket not in connected state");
		}

		byte_buffer::reader reader = bb.read();
		byte_t* msg = new byte_t[bb.size()];
		reader.get_array(msg, bb.size());
		int to_send = bb.size();
		int already_sent = 0;

		while(already_sent < to_send) {
			int tx = send(client_socket, msg+already_sent, to_send-already_sent, 0);
			if(tx == INVALID_SOCKET) break;
			already_sent += tx;
		}
		delete msg;

	}

	bool ClientSocket::connected() { return cs_->connected_(); }

	void ClientSocket::begin_connect(std::string host, uint16_t port, int usec) {
		cs_->begin_connect(host, port, usec);
	}

	void ClientSocket::begin_send(const byte_buffer &bb) {
		cs_->begin_send(bb);
	}

	ClientSocket::ClientSocket() {
		cs_ = new ClientSocketImpl(this);
	}

	ClientSocket::ClientSocket(int ext) {
		cs_ = new ClientSocketImpl(this, ext);
	}

	ClientSocket::~ClientSocket() {
		delete cs_;
	}
}