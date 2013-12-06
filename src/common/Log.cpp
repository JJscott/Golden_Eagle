#include "Log.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace ambition {

	namespace termcolor {

#ifdef AMBITION_NO_TERMCOLOR
		// Reset Color
		std::ostream & reset(std::ostream &o) { return o; }

		// Regular Colors
		std::ostream & black(std::ostream &o) { return o; }
		std::ostream & red(std::ostream &o) { return o; }
		std::ostream & green(std::ostream &o) { return o; }
		std::ostream & yellow(std::ostream &o) { return o; }
		std::ostream & blue(std::ostream &o) { return o; }
		std::ostream & purple(std::ostream &o) { return o; }
		std::ostream & cyan(std::ostream &o) { return o; }
		std::ostream & white(std::ostream &o) { return o; }

		// Bold Colors
		std::ostream & boldBlack(std::ostream &o) { return o; }
		std::ostream & boldRed(std::ostream &o) { return o; }
		std::ostream & boldGreen(std::ostream &o) { return o; }
		std::ostream & boldYellow(std::ostream &o) { return o; }
		std::ostream & boldBlue(std::ostream &o) { return o; }
		std::ostream & boldPurple(std::ostream &o) { return o; }
		std::ostream & boldCyan(std::ostream &o) { return o; }
		std::ostream & boldWhite(std::ostream &o) { return o; }

#elif defined(_WIN32)
		// use windows console manip

		namespace {
			HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
			HANDLE hstderr = GetStdHandle(STD_ERROR_HANDLE);

			CONSOLE_SCREEN_BUFFER_INFO *csbi_out = nullptr;
			CONSOLE_SCREEN_BUFFER_INFO *csbi_err = nullptr;

			void checkConsoleDefaultsSaved() {
				// save the original values for reset
				if (csbi_out == nullptr) {
					csbi_out = new CONSOLE_SCREEN_BUFFER_INFO;
					GetConsoleScreenBufferInfo(hstdout, csbi_out);
				}
				if (csbi_err == nullptr) {
					csbi_err = new CONSOLE_SCREEN_BUFFER_INFO;
					GetConsoleScreenBufferInfo(hstderr, csbi_err);
				}
			}

			void resetTextAttribute(std::ostream &o) {
				checkConsoleDefaultsSaved();
				if (&o == &std::cout) {
					SetConsoleTextAttribute(hstdout, csbi_out->wAttributes);
				}
				if (&o == &std::cerr) {
					SetConsoleTextAttribute(hstderr, csbi_err->wAttributes);
				}
			}

			void setTextColor(std::ostream &o, WORD w) {
				checkConsoleDefaultsSaved();
				// set color, preserving current background
				// apparently i dont need to explicitly flush before setting the color
				if (&o == &std::cout) {
					w = w & 0x0F;
					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(hstdout, &csbi);
					w = (csbi.wAttributes & 0xF0) | w;
					SetConsoleTextAttribute(hstdout, w);
				}
				if (&o == &std::cerr) {
					w = w & 0x0F;
					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(hstderr, &csbi);
					w = (csbi.wAttributes & 0xF0) | w;
					SetConsoleTextAttribute(hstderr, w);
				}
			}

		}

		// Reset Color
		std::ostream & reset(std::ostream &o) { resetTextAttribute(o); return o; }

		// Regular Colors
		std::ostream & black(std::ostream &o) { setTextColor(o, 0); return o; }
		std::ostream & red(std::ostream &o) { setTextColor(o, 4); return o; }
		std::ostream & green(std::ostream &o) { setTextColor(o, 2); return o; }
		std::ostream & yellow(std::ostream &o) { setTextColor(o, 6); return o; }
		std::ostream & blue(std::ostream &o) { setTextColor(o, 1); return o; }
		std::ostream & purple(std::ostream &o) { setTextColor(o, 5); return o; }
		std::ostream & cyan(std::ostream &o) { setTextColor(o, 3); return o; }
		std::ostream & white(std::ostream &o) { setTextColor(o, 7); return o; }

		// Bold Colors
		std::ostream & boldBlack(std::ostream &o) { setTextColor(o, 8); return o; }
		std::ostream & boldRed(std::ostream &o) { setTextColor(o, 12); return o; }
		std::ostream & boldGreen(std::ostream &o) { setTextColor(o, 10); return o; }
		std::ostream & boldYellow(std::ostream &o) { setTextColor(o, 14); return o; }
		std::ostream & boldBlue(std::ostream &o) { setTextColor(o, 9); return o; }
		std::ostream & boldPurple(std::ostream &o) { setTextColor(o, 13); return o; }
		std::ostream & boldCyan(std::ostream &o) { setTextColor(o, 11); return o; }
		std::ostream & boldWhite(std::ostream &o) { setTextColor(o, 15); return o; }

#else
		// use unix escape codes

		// Reset Color
		std::ostream & reset(std::ostream &o) { o << "\033[0m"; return o; }

		// Regular Colors
		std::ostream & black(std::ostream &o) { o << "\033[0;30m"; return o; }
		std::ostream & red(std::ostream &o) { o << "\033[0;31m"; return o; }
		std::ostream & green(std::ostream &o) { o << "\033[0;32m"; return o; }
		std::ostream & yellow(std::ostream &o) { o << "\033[0;33m"; return o; }
		std::ostream & blue(std::ostream &o) { o << "\033[0;34m"; return o; }
		std::ostream & purple(std::ostream &o) { o << "\033[0;35m"; return o; }
		std::ostream & cyan(std::ostream &o) { o << "\033[0;36m"; return o; }
		std::ostream & white(std::ostream &o) { o << "\033[0;37m"; return o; }

		// Bold Colors
		std::ostream & boldBlack(std::ostream &o) { o << "\033[1;30m"; return o; }
		std::ostream & boldRed(std::ostream &o) { o << "\033[1;31m"; return o; }
		std::ostream & boldGreen(std::ostream &o) { o << "\033[1;32m"; return o; }
		std::ostream & boldYellow(std::ostream &o) { o << "\033[1;33m"; return o; }
		std::ostream & boldBlue(std::ostream &o) { o << "\033[1;34m"; return o; }
		std::ostream & boldPurple(std::ostream &o) { o << "\033[1;35m"; return o; }
		std::ostream & boldCyan(std::ostream &o) { o << "\033[1;36m"; return o; }
		std::ostream & boldWhite(std::ostream &o) { o << "\033[1;37m"; return o; }
#endif

	}

	const char * const Log::s_level_names[] = { "Information", "Warning", "Error", "Critical", "NOPE", "IDGAF" };

	std::unordered_set<LogOutput *> * Log::s_outputs = new std::unordered_set<LogOutput *>();
	LogOutput * const Log::s_cout = new ColoredStreamLogOutput(&std::cout, true);
	LogOutput * const Log::s_cerr = new ColoredStreamLogOutput(&std::cerr, false);

}