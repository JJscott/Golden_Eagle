#include "Log.hpp"

namespace ambition {

	namespace termcolor {

		// Unix Terminal Colors

#ifdef _WIN32
		// Reset Color
		const char *ColorOff = "";

		// Regular Colors
		const char *Black = "";
		const char *Red = "";
		const char *Green = "";
		const char *Yellow = "";
		const char *Blue = "";
		const char *Purple = "";
		const char *Cyan = "";
		const char *White = "";

		// Bold Colors
		const char *BBlack = "";
		const char *BRed = "";
		const char *BGreen = "";
		const char *BYellow = "";
		const char *BBlue = "";
		const char *BPurple = "";
		const char *BCyan = "";
		const char *BWhite = "";
#else
		// Reset Color
		const char *ColorOff = "\033[0m";

		// Regular Colors
		const char *Black = "\033[0;30m";
		const char *Red = "\033[0;31m";
		const char *Green = "\033[0;32m";
		const char *Yellow = "\033[0;33m";
		const char *Blue = "\033[0;34m";
		const char *Purple = "\033[0;35m";
		const char *Cyan = "\033[0;36m";
		const char *White = "\033[0;37m";

		// Bold Colors
		const char *BBlack = "\033[1;30m";
		const char *BRed = "\033[1;31m";
		const char *BGreen = "\033[1;32m";
		const char *BYellow = "\033[1;33m";
		const char *BBlue = "\033[1;34m";
		const char *BPurple = "\033[1;35m";
		const char *BCyan = "\033[1;36m";
		const char *BWhite = "\033[1;37m";
#endif

	}

	const char * const Log::s_level_names[] = { "Information", "Warning", "Error", "Critical", "NOPE", "IDGAF" };

	std::unordered_set<LogOutput *> * Log::s_outputs = new std::unordered_set<LogOutput *>();
	LogOutput * const Log::s_cout = new CoutLogOutput();

}