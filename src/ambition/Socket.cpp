#include "Socket.hpp"
#include "Log.hpp"

#include <cstdio>

	namespace ambition {

	#ifndef _WIN32
		typedef int SOCKET;
		#define INVALID_SOCKET -1
		#include <arpa/inet.h>
		#include <unistd.h>
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <errno.h>
		#include <netinet/in.h>
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

		#include <winsock.h>
		typedef int socklen_t;
	#endif

	#define FD_ZERO_F FD_ZERO
	#define FD_ISSET_F FD_ISSET

	class ListenSocket::ListenSocketImpl {
		sockaddr_in serveraddr;
		sockaddr_in clientaddr;
		fd_set master;
		fd_set read_fds;
		SOCKET fdmax;
		SOCKET listener;
		SOCKET newfd;

		uint16_t listen_port_impl = -1;

		char buf[2048];
		int nBytes;

		int yes = 1;
		socklen_t addrlen;

	public:
		void init();
		uint16_t listen_port() { return listen_port_impl; }
	};

	ListenSocket::ListenSocket() {
		lsock = new ListenSocketImpl;
	}

	ListenSocket::~ListenSocket() {
		delete lsock;
	}

	void ListenSocket::init() { lsock->init(); }

	uint16_t ListenSocket::listen_port() { return lsock->listen_port(); }

	void ListenSocket::ListenSocketImpl::init() {
		FD_ZERO_F(&master);
		FD_ZERO_F(&read_fds);

	#ifdef _WIN32
		WSAData data;
		WSAStartup(MAKEWORD(1, 1), &data);
	#endif

		if((listener = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			throw "unable to socket()";

		printf("listener: %d errno: %d\n", listener, errno);

		if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == INVALID_SOCKET) {
			printf("Error#: %d\n", errno);
			throw "unable to setup socket";
		}

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = INADDR_ANY;
		serveraddr.sin_port = 0;
		
		if((bind(listener, (sockaddr*)&serveraddr, sizeof(serveraddr))) == INVALID_SOCKET)
			throw "unable to bind()";

		// we've bound - report the port we have bound to

		listen_port_impl = ntohs(serveraddr.sin_port);
		log("Socket") % 0 << "Bound to port: " << ntohs(serveraddr.sin_port);

		if(listen(listener, 10) == INVALID_SOCKET)
			throw "unable to listen()";

		FD_SET_F(listener, &master);
		fdmax = listener;

		for (;;) {
			read_fds = master;
			printf("select()'ing\n");
			if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
				throw "select() error";
			}

			for(SOCKET i = 0; i <= fdmax; i++) {
				if(FD_ISSET(i, &read_fds))
				{
					if(i == listener) {
						addrlen = sizeof(clientaddr);
						if((newfd = accept(listener, (sockaddr*)&clientaddr, &addrlen)) == INVALID_SOCKET)
							throw "accept() error";
						// printf("new socket: %d\tmaster: %d\n", newfd, master);
						FD_SET_F(newfd, &master);
						if(newfd > fdmax)
							fdmax = newfd;

						printf("new connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
					} else {
						if((nBytes = recv(i, buf, sizeof(buf), 0)) <= 0)
						{
							if(nBytes == 0)
								printf("socket %d hung up\n", i);
							else
								throw "recv error";
	#ifndef _WIN32
							close(i);
	#else
							closesocket(i);
	#endif
							FD_CLR_F(i, &master);
						}
						else {
							for(SOCKET j = 0; j <= fdmax; j++) {
								if(FD_ISSET_F(j, &master)) {
									if(j != listener && j != i) {
										if(send(j, buf, nBytes, 0) == INVALID_SOCKET)
											throw "send() failed";
									}
								}
							}
						}
					}
				}
			}
		}
	}
}