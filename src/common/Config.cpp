#include "Config.hpp"

#include <fstream>

string trim(const string&, const char* = "\t\r\n");

namespace ambition {

	Config::Config(const string& f) {
		ifstream file(f.c_str());

		string line, name, value, inSection;
		int posEqual;

		while(getline(file, line)) {
			if(!line.length() || line[0] == '#') continue;

			if(line[0] == '[') {
				inSection = trim(line.substr(1, line.find(']')-1));
				sections.push_back(inSection);
				continue;
			}

			posEqual = line.find('=');
			name = trim(line.substr(0, posEqual));
			value = trim(line.substr(posEqual+1));

			content[inSection+"/"+name] = value;
		}
	}

	vector<string> Config::getSections() {
		return sections;
	}

	const string& Config::get(const string& section, const string& name) const {
		map<string, string>::const_iterator it = content.find(section + '/' + name);
		if(it == content.end()) throw "section/value not found";
		return it->second;
	}

	const string& Config::get(const string& section, const string& name, const string& value) {
		try {
			return get(section, name);
		} catch(const char*) {
			return content.insert(make_pair(section + '/' + name, value)).first->second;
		}
		throw "value cannot be set";
	}
}

string trim(const string& source, const char* delims) {
	string result(source);
	string::size_type index = result.find_last_not_of(delims);
	if(index != string::npos)
		result.erase(++index);

	index = result.find_first_not_of(delims);
	if(index != string::npos)
		result.erase(0, index);
	else
		result.erase();
	return result;
}