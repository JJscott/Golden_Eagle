#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class ListenSocket {
	sockaddr_in serveraddr;
	sockaddr_in clientaddr;
	fd_set master;
	fd_set read_fds;
	int fdmax;
	int listener;
	int newfd;

	char buf[2048];
	int nBytes;

	int yes = 1;
	socklen_t addrlen;

public:

	ListenSocket() {}
	~ListenSocket() {}

	void init() {
		FD_ZERO(&master);
		FD_ZERO(&read_fds);

		if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			throw "unable to socket()";

		printf("listener: %d errno: %d\n", listener, errno);

		if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			printf("Error#: %d\n", errno);
			throw "unable to setup socket";
		}

		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = INADDR_ANY;
		serveraddr.sin_port = htons(2020);
		memset(&(serveraddr.sin_zero), '\0', 8);

		if(bind(listener, (sockaddr*)&serveraddr, sizeof(serveraddr)) == -1)
			throw "unable to bind()";

		if(listen(listener, 10) == -1)
			throw "unable to listen()";

		FD_SET(listener, &master);
		fdmax = listener;

		while(1) {
			read_fds = master;
			printf("select()'ing\n");
			if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
				throw "select() error";
			}

			for(int i = 0; i <= fdmax; i++) {
				if(FD_ISSET(i, &read_fds))
				{
					if(i == listener) {
						addrlen = sizeof(clientaddr);
						if((newfd = accept(listener, (sockaddr*)&clientaddr, &addrlen)) == -1)
							throw "accept() error";
						printf("new socket: %d\tmaster: %d\n", newfd, master);
						FD_SET(newfd, &master);
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
							close(i);
							FD_CLR(i, &master);
						}
						else {
							for(int j = 0; j <= fdmax; j++) {
								if(FD_ISSET(j, &master)) {
									if(j != listener && j != i) {
										if(send(j, buf, nBytes, 0) == -1)
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
};