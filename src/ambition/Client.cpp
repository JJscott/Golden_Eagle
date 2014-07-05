
#include "Client.hpp"
#include "Log.hpp"

using namespace std;

namespace ambition {
	void Client::target(std::string hostname, uint16_t port) {

	}

	void Client::block_until_connected(int ms_timeout) {

	}

	bool Client::connected() {
		return false;
	}

	int Client::get_server_version() {
		return 0;
	}

	int Client::get_version() {
		return 1;
	}
}