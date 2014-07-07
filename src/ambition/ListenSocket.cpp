#include "ListenSocket.hpp"
#include "Log.hpp"
#include "Error.hpp"


#include <thread>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <winsock.h>
using socklen_t = int;
#else
using SOCKET = int;
#define INVALID_SOCKET -1
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#define FD_CLR_F FD_CLR
#define FD_SET_F FD_SET
#define	closesocket(i) close(i)
#endif

namespace ambition {

	#ifdef _WIN32
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

		
		
	#endif

	#define FD_ZERO_F FD_ZERO
	#define FD_ISSET_F FD_ISSET

	class ListenSocket::ListenSocketImpl {
	public:
		ListenSocketImpl(ListenSocket*);
		sockaddr_in serveraddr;
		sockaddr_in clientaddr;
		fd_set master;
		fd_set read_fds;
		SOCKET fdmax;
		SOCKET listener;
		ListenSocket* outer;

		uint16_t listen_port_impl = -1;

		char buf[2048];
		int nBytes;

		int yes = 1;
		socklen_t addrlen;
		std::thread* twork;
		std::map<SOCKET, ClientSocket*> cons;
		static void work(ListenSocket::ListenSocketImpl*);

	public:
		void init();
		uint16_t listen_port() { return listen_port_impl; }
	};

	ListenSocket::ListenSocket() {
		lsock = new ListenSocketImpl(this);
		init();
	}

	ListenSocket::~ListenSocket() {
		delete lsock;
	}

	void ListenSocket::init() { lsock->init(); }

	uint16_t ListenSocket::listen_port() { return lsock->listen_port(); }

	void ListenSocket::ListenSocketImpl::work(ListenSocket::ListenSocketImpl* target) {
		int rv;
		while(true) {
			target->read_fds = target->master;
			rv = select(target->fdmax+1, &target->read_fds, NULL, NULL, NULL);

			if(rv == INVALID_SOCKET) {
				network_error ne(error::neterr_select_failure, "General select() error");
				ne.error_no = errno;
				ne.error_message = strerror(errno);
				throw ne;
			}			

			for(SOCKET i = 0; i <= target->fdmax; i++) {
				if(FD_ISSET(i, &target->read_fds))
				{
					if(i == target->listener) {
						target->addrlen = sizeof(target->clientaddr);
						SOCKET newfd;
						if((newfd = accept(target->listener, (sockaddr*)&target->clientaddr, &target->addrlen)) == INVALID_SOCKET)
							throw "accept() error";

						FD_SET_F(newfd, &target->master);
						if(newfd > target->fdmax)
							target->fdmax = newfd;

						SocketResult sr;
						sr.success = true;
						ClientSocket* cs_new = new ClientSocket(newfd);
						target->cons[newfd] = cs_new;
						sr.client = cs_new;
						target->outer->on_accepted.notify(sr);
					} else {
						int rx = recv(i, target->buf, sizeof(target->buf), 0);
						if(rx == INVALID_SOCKET) {
							closesocket(i);
							FD_CLR_F(i, &target->master);
							throw "recv error";

						}
						else if(rx == 0) {
							printf("socket %d hung up\n", i);
							closesocket(i);
							FD_CLR_F(i, &target->master);
						} else {
							// recv
							std::map<SOCKET, ClientSocket*>::const_iterator cif = target->cons.find(i);
							if(cif == target->cons.end()) {
								// oh snap!
							} else {
								SocketResult sr;
								sr.success = true;
								sr.data = new ByteBuffer(target->buf, rx);
								sr.client = cif->second;
								cif->second->on_recieved.notify(sr);
							}
						}
					}
				}
			}
		}
	}

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
		serveraddr.sin_port = htons(8119);
		
		if((bind(listener, (sockaddr*)&serveraddr, sizeof(serveraddr))) == INVALID_SOCKET)
			throw "unable to bind()";

		// we've bound - report the port we have bound to

		listen_port_impl = ntohs(serveraddr.sin_port);
		std::cout << "bound to: " << ntohs(serveraddr.sin_port) << std::endl;
		log("Socket") % 0 << "Bound to port: " << ntohs(serveraddr.sin_port);

		if(listen(listener, 10) == INVALID_SOCKET)
			throw "unable to listen()";

		FD_SET_F(listener, &master);
		fdmax = listener;

		twork = new std::thread(work, this);		
	}

	ListenSocket::ListenSocketImpl::ListenSocketImpl(ListenSocket* o) : outer(o) {}
}
