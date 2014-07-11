#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/HTTP.hpp>

#include <CryptoPP/osrng.h>

using namespace ambition;

int main(int argc, char** argv) {
	CryptoPP::AutoSeededRandomPool rand;
	log("test") << rand.GenerateWord32();

	http_connection con("google.co.nz");
	http_request* req = new http_get_request("/");
	con.open();
	con.execute(req);
	con.close();
	req->on_complete.wait();
	std::cout << req->reply()->body_s() << std::endl << std::endl;
}
