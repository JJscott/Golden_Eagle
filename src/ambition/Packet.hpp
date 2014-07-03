#ifndef PACKET_HPP
#define PACKET_HPP

#include <queue>

class Packet {
	std::queue<char> bytestream;
public:
	int size() { return bytestream.size(); }
	void add(char b) {
		bytestream.push(b);
	}

	char peek_byte() {
		return bytestream.front();
	}

	char get_byte() {
		char t = bytestream.front();
		bytestream.pop();
		return t;
	}

	void add(char* bts, size_t n) {
		for(size_t i = 0; i < n; i++) {
			bytestream.push(bts[i]);
		}
	}

	char* peek_bytes(size_t n) {
		// todo
		return NULL;
	}

	char* get_bytes(size_t n) {
		if(n > bytestream.size()) return NULL;
		char* ret = new char[n];
		for(size_t i = 0; i < n; i++) {
			ret[i] = bytestream.front();
			bytestream.pop();
		}
		return ret;
	}
};

#endif 