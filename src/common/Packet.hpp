#ifndef PACKET_HPP
#define PACKET_HPP

#include <queue>

class Packet {
	std::queue<char> bytestream;
public:
	void add(char b) {
		bytestream.push(b);
	}

	char get_byte() {
		char t = bytestream.front();
		bytestream.pop();
		return t;
	}
};

#endif