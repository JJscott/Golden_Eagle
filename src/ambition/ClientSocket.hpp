#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <thread>
#include <cstring>

#include "ambition/Event.hpp"

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
	struct SocketResult {
		bool success;
	};

	class ClientSocket {

		
	public:
		Event<SocketResult> on_connected;

		fd_set fdset;
		SOCKET client_socket;
		timeval tv;
		std::thread* worker;

		ClientSocket() { 
			client_socket = socket(AF_INET, SOCK_STREAM, 0);
			if(client_socket == INVALID_SOCKET) {
				throw std::runtime_error("Unable to create socket");
			}
			fcntl(client_socket, F_SETFL, O_NONBLOCK);
			if(client_socket == INVALID_SOCKET) {
				throw std::runtime_error("Unable to set non-blocking mode on socket");
			}
			FD_ZERO(&fdset);
    		FD_SET(client_socket, &fdset);
		}

		static void work_thread(ClientSocket* target) {
			int rv;
			while(true) {
				rv = select(target->client_socket+1,  NULL, &target->fdset, NULL, NULL);
				if (rv >= 1) {
        			int so_error;
        			socklen_t slen = sizeof(so_error);
        			getsockopt(target->client_socket, SOL_SOCKET, SO_ERROR, &so_error, &slen);

        			SocketResult sr;
        			sr.success = (so_error == 0);

        			target->on_connected.notify(sr);
        			if(!sr.success) return;
        		}
        		return;
			}
		}

		void begin_connect(std::string hostname, uint16_t pt, int us_timeout) {
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
				throw std::runtime_error("Unable to resolve hostname / port");
			}

			rv = connect(client_socket, rp->ai_addr, rp->ai_addrlen);
			worker = new std::thread(work_thread, this);
		}
	};
}

#endif
