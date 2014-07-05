#include "gtest/gtest.h"
#include "ambition/Server.hpp"
#include "ambition/Client.hpp"

// TODO: Write tests for Client v Server coms in here.
TEST(Comms, Hello) {
	Client c;
	Server s;
	c.target("localhost", s.listen_port());
}