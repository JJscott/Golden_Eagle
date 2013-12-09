#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <vector>

namespace ambition {
	class Config {
		std::map<std::string, std::string> content;
		std::vector<std::string> sections;

	public:
		Config(const std::string& file);
		std::vector<std::string> getSections();
		const std::string& get(const std::string& section, const std::string& entry) const;
		const std::string& get(const std::string&, const std::string&, const std::string&);
	};
}

#endif