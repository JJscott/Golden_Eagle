#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <vector>

using namespace std;

namespace ambition {
	class Config {
		map<string, string> content;
		vector<string> sections;

	public:
		Config(const string& file);
		vector<string> getSections();
		const string& get(const string& section, const string& entry) const;
		const string& get(const string&, const string&, const string&);
	};
}

#endif