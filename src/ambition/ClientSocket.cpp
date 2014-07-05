#include <thread>
#include <cstring>

#include "ClientSocket.hpp"

#ifndef _WIN32
	typedef int SOCKET;
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
		fd_set fdset;
		SOCKET client_socket;
		timeval tv;
		std::thread* worker;
		ClientSocket* outer;
	public:
		ClientSocketImpl(ClientSocket*);
		static void work_thread(ClientSocketImpl* target);
		void begin_connect(std::string, uint16_t, int);
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
		ioctlsocket(client_socket,FIONBIO,&iMode);
		#else
		fcntl(client_socket, F_SETFL, O_NONBLOCK);
		#endif

		if(client_socket == INVALID_SOCKET) {
			network_error ne(error::neterr_socket_set_blocking_failure, "Unable to set socket blocking mode");
			ne.error_no = errno;
			ne.error_message = strerror(errno);
			throw ne;
		}
		FD_ZERO_F(&fdset);
		FD_SET_F(client_socket, &fdset);
	}

	void ClientSocket::ClientSocketImpl::work_thread(ClientSocketImpl* target) {
		int rv;
		while(true) {
			rv = select(target->client_socket+1,  NULL, &target->fdset, NULL, NULL);
			if(rv == INVALID_SOCKET) {
				network_error ne(error::neterr_select_failure, "General select() error");
				ne.error_no = errno;
				ne.error_message = strerror(errno);
				throw ne;
			}
			if (rv >= 1) {
				char so_error;
				socklen_t slen = sizeof(so_error);
				getsockopt(target->client_socket, SOL_SOCKET, SO_ERROR, &so_error, &slen);

				SocketResult sr;
				sr.success = (so_error == 0);

				target->outer->on_connected.notify(sr);
				if(!sr.success) return;
			}
			return;
		}
	}

	void ClientSocket::ClientSocketImpl::begin_connect(std::string hostname, uint16_t pt, int us_timeout) {
		// let's a get a new thread, to deal with the socket polling
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

	void ClientSocket::begin_connect(std::string host, uint16_t port, int usec) {
		cs_->begin_connect(host, port, usec);
	}

	ClientSocket::ClientSocket() {
		cs_ = new ClientSocketImpl(this);
	}

	ClientSocket::~ClientSocket() {
		delete cs_;
	}
}