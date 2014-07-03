#ifndef SERVER_HPP
#define SERVER_HPP

#include "ambition/Log.hpp"

using namespace ambition;

class Server {
	bool isPublic = false;
public:
	Server(bool);
	void start();
};

#endif