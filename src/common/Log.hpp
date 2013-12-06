#ifndef AMBITION_LOG_HPP
#define AMBITION_LOG_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace ambition {

	namespace termcolor {

		// Terminal Color Manipulators (use these like std::endl on std::cout and std::cerr)

		// Reset Color
		std::ostream & reset(std::ostream &);
		
		// Regular Colors
		std::ostream & black(std::ostream &);
		std::ostream & red(std::ostream &);
		std::ostream & green(std::ostream &);
		std::ostream & yellow(std::ostream &);
		std::ostream & blue(std::ostream &);
		std::ostream & purple(std::ostream &);
		std::ostream & cyan(std::ostream &);
		std::ostream & white(std::ostream &);
		
		// Bold Colors
		std::ostream & boldBlack(std::ostream &);
		std::ostream & boldRed(std::ostream &);
		std::ostream & boldGreen(std::ostream &);
		std::ostream & boldYellow(std::ostream &);
		std::ostream & boldBlue(std::ostream &);
		std::ostream & boldPurple(std::ostream &);
		std::ostream & boldCyan(std::ostream &);
		std::ostream & boldWhite(std::ostream &);

	}

	class LogOutput {
	private:
		LogOutput(const LogOutput &rhs) = delete;
		LogOutput & operator=(const LogOutput &rhs) = delete;
		
		unsigned m_min_level = 0;
		bool m_mute = false;

	protected:
		virtual void write_impl(unsigned level, const std::string &str) = 0;

	public:
		LogOutput(bool mute_ = false) : m_mute(mute_) { }

		inline unsigned getMinLevel() {
			return m_min_level;
		}

		inline void setMinLevel(unsigned level) {
			m_min_level = level;
		}

		inline bool getMute() {
			return m_mute;
		}

		inline void setMute(bool b) {
			m_mute = b;
		}

		virtual void write(unsigned level, const std::string &str) {
			if (!m_mute && level >= m_min_level) write_impl(level, str);
		}

		virtual ~LogOutput() { }

	};

	class Log {
	private:
		Log() = delete;
		Log(const Log &rhs) = delete;
		Log & operator=(const Log &rhs) = delete;

		static const char * const s_level_names[];

		static std::unordered_set<LogOutput *> * s_outputs;

		// cout starts muted, cerr starts non-muted
		static LogOutput * const s_cout;
		static LogOutput * const s_cerr;

	public:
		static const unsigned information = 0;
		static const unsigned warning = 1;
		static const unsigned error = 2;
		static const unsigned critical = 3;
		static const unsigned nope = 4;
		static const unsigned idgaf = 5;
		static const unsigned max = idgaf;

		static inline void write(unsigned level, const std::string &source, const std::string &msg) {
			level = level > max ? max : level;
			// TODO better format maybe?
			// Wed May 30 12:25:03 2012 [System] Error : IT BROEK

			std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::ostringstream ss;

			std::string ts = std::string(ctime(&tt));
			ts.pop_back();

			ss << ts;
			ss << " [" << std::setw(15) << source << "] ";
			ss << std::setw(11) << s_level_names[level] << " : ";
			ss << msg;
			ss << '\n';

			// write to stderr and stdout
			s_cerr->write(level, ss.str());
			s_cout->write(level, ss.str());

			// write to all others
			for (LogOutput *out : *s_outputs) {
				out->write(level, ss.str());
			}
		}

		static inline void addOutput(LogOutput *out) {
			s_outputs->insert(out);
		}

		static inline void removeOutput(LogOutput *out) {
			s_outputs->erase(out);
		}

		static inline LogOutput * getStandardOut() {
			return s_cout;
		}

		static inline LogOutput * getStandardErr() {
			return s_cerr;
		}

	};

	class StreamLogOutput : public LogOutput {
	private:
		std::ostream *m_out;

	protected:
		virtual void write_impl(unsigned level, const std::string &str) override {
			(*m_out) << str;
		}

		inline std::ostream *getStream() {
			return m_out;
		}

	public:
		explicit inline StreamLogOutput(std::ostream *out_, bool mute_ = false) : LogOutput(mute_), m_out(out_) { }
	};

	class ColoredStreamLogOutput : public StreamLogOutput {
	protected:
		virtual void write_impl(unsigned level, const std::string &str) override {
			std::ostream &out = *getStream();
			using namespace std;
			switch (level) {
			case Log::warning:
				out << termcolor::yellow; break;
			case Log::error:
				out << termcolor::boldRed; break;
			case Log::critical:
				out << termcolor::red; break;
			case Log::nope:
				out << termcolor::purple; break;
			case Log::idgaf:
				out << termcolor::blue; break;
			default:
				out << termcolor::reset;
			}
			out << str;
			out << termcolor::reset;
			out.flush();
		}

	public:
		explicit inline ColoredStreamLogOutput(std::ostream *out_, bool mute_ = false) : StreamLogOutput(out_, mute_) { }
	};

	class FileLogOutput : public StreamLogOutput {
	private:
		// dtor should take care of closing this
		std::ofstream m_out;

	public:
		explicit inline FileLogOutput(const std::string &fname_, std::ios_base::openmode mode_ = std::ios_base::trunc, bool mute_ = false) : StreamLogOutput(&m_out, mute_) {
			m_out.open(fname_, mode_);
		}
	};
	
	template <typename CharT>
	class basic_logstream : public std::basic_ostream<CharT> {
	private:
		std::string m_source;
		unsigned m_level;
		std::basic_stringbuf<CharT> m_buf;
		bool m_write;

		basic_logstream(const basic_logstream &rhs) = delete;
		basic_logstream & operator=(const basic_logstream &rhs) = delete;

	public:
		basic_logstream(const std::string &source_) : 
			std::basic_ostream<CharT>(&m_buf),
			m_source(source_),
			m_level(0),
			m_write(true) { }

		// copy ctor takes over responsibility for writing to log
		basic_logstream(basic_logstream &&rhs) : m_buf(rhs.m_buf.str()), m_write(true) {
			rhs.m_write = false;
		}

		// set level
		basic_logstream & operator%(unsigned level) {
			m_level = level;
			return *this;
		}

		~basic_logstream() {
			if (m_write) {
				Log::write(m_level, m_source, m_buf.str());
			}
		}

	};

	typedef basic_logstream<char> logstream;

	inline logstream log(const std::string &source) {
		return logstream(source);
	}

	inline logstream log() {
		return logstream("Global");
	}

}

#endif // AMBITION_LOG_HPP