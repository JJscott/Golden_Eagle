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
#include <cstdio>

namespace ambition {

	namespace termcolor {

		// Unix Terminal Colors

		// Reset Color
		extern const char *ColorOff;
		
		// Regular Colors
		extern const char *Black;
		extern const char *Red;
		extern const char *Green;
		extern const char *Yellow;
		extern const char *Blue;
		extern const char *Purple;
		extern const char *Cyan;
		extern const char *White;
		
		// Bold Colors
		extern const char *BBlack;
		extern const char *BRed;
		extern const char *BGreen;
		extern const char *BYellow;
		extern const char *BBlue;
		extern const char *BPurple;
		extern const char *BCyan;
		extern const char *BWhite;

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
		LogOutput() { }

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
		static LogOutput * const s_cout;

	public:
		static const unsigned information = 0;
		static const unsigned warning = 1;
		static const unsigned error = 2;
		static const unsigned critical = 3;
		static const unsigned nope = 4;
		static const unsigned idgaf = 5;
		static const unsigned max = idgaf;

		static void write(unsigned level, const std::string &source, const std::string &msg) {
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
			ss << std::endl;

			// write to stdout
			s_cout->write(level, ss.str());

			// write to all others
			for (LogOutput *out : *s_outputs) {
				out->write(level, ss.str());
			}
		}

		static void addOutput(LogOutput *out) {
			s_outputs->insert(out);
		}

		static void removeOutput(LogOutput *out) {
			s_outputs->erase(out);
		}

		static LogOutput * getStandardOut() {
			return s_cout;
		}

	};

	class StreamLogOutput : public LogOutput {
	private:
		std::ostream *m_out;

	protected:
		virtual void write_impl(unsigned level, const std::string &str) override {
			(*m_out) << str;
		}

	public:
		explicit inline StreamLogOutput(std::ostream *out_) : m_out(out_) { }
	};

	class CoutLogOutput : public StreamLogOutput {
	protected:
		virtual void write_impl(unsigned level, const std::string &str) override {
			using namespace std;
			switch (level) {
			case Log::warning:
				cout << termcolor::Yellow; break;
			case Log::error:
				cout << termcolor::BRed; break;
			case Log::critical:
				cout << termcolor::Red; break;
			case Log::nope:
				cout << termcolor::Purple; break;
			case Log::idgaf:
				cout << termcolor::Blue; break;
			default:
				cout << termcolor::ColorOff;
			}
			cout << str;
			cout << termcolor::ColorOff;
		}

	public:
		explicit inline CoutLogOutput() : StreamLogOutput(&std::cout) { }
	};

	class FileLogOutput : public StreamLogOutput {
	private:
		// dtor should take care of closing this
		std::ofstream m_out;

	public:
		explicit inline FileLogOutput(const std::string &fname_, std::ios_base::openmode mode_ = std::ios_base::app) : StreamLogOutput(&m_out) {
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