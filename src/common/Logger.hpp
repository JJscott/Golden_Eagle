#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <vector>
#include <iostream>

class Logger {
	std::vector<std::ostream*> outputStreams;

	public:
		void addOutputStream(std::ostream&);
		void addOutputFile(const char*);

		void writeString(const char*);
		
};

#endif