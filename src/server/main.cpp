#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <ambition/HTTP.hpp>

using namespace ambition;

int main(int argc, char** argv) {
	http_connection con("google.co.nz");
	http_request* req = new http_get_request("/");
	con.open();
	con.execute(req);
	con.close();
	req->on_complete.wait();
	std::cout << req->reply()->body_s() << std::endl << std::endl;
}