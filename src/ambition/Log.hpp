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
#include <mutex>

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
		
		unsigned m_verbosity = 2;
		bool m_mute = false;

	protected:
		// this is responsible for newlines
		virtual void write_impl(unsigned verbosity, unsigned type, const std::string &str) = 0;

	public:
		LogOutput(bool mute_ = false) : m_mute(mute_) { }

		inline unsigned verbosity() {
			return m_verbosity;
		}

		inline void verbosity(unsigned v) {
			m_verbosity = v;
		}

		inline bool mute() {
			return m_mute;
		}

		inline void mute(bool b) {
			m_mute = b;
		}

		inline void write(unsigned verbosity, unsigned type, const std::string &str) {
			if (!m_mute && verbosity <= m_verbosity) write_impl(verbosity, type, str);
		}

		virtual ~LogOutput() { }

	};

	class Log {
	private:
		Log() = delete;
		Log(const Log &rhs) = delete;
		Log & operator=(const Log &rhs) = delete;

		static std::mutex m_mutex;
		static std::unordered_set<LogOutput *> m_outputs;

		// cout starts muted, cerr starts non-muted
		static LogOutput * const m_cout;
		static LogOutput * const m_cerr;

	public:
		static const unsigned information = 0;
		static const unsigned warning = 1;
		static const unsigned error = 2;

		static void write(unsigned verbosity, unsigned type, const std::string &source, const std::string &msg);

		static inline void addOutput(LogOutput &out) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_outputs.insert(&out);
		}

		static inline void removeOutput(LogOutput &out) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_outputs.erase(&out);
		}

		static inline LogOutput & stdOut() {
			return *m_cout;
		}

		static inline LogOutput & stdErr() {
			return *m_cerr;
		}

	};

	// log output that writes to a std::ostream
	class StreamLogOutput : public LogOutput {
	private:
		std::ostream *m_out;

	protected:
		virtual void write_impl(unsigned, unsigned, const std::string &str) override {
			(*m_out) << str << std::endl;
		}

		inline std::ostream & getStream() {
			return *m_out;
		}

	public:
		explicit inline StreamLogOutput(std::ostream *out_, bool mute_ = false) : LogOutput(mute_), m_out(out_) { }
	};

	// log output for writing to std::cout or std::cerr (as they are the only streams with reliable color support)
	class ColoredStreamLogOutput : public StreamLogOutput {
	protected:
		virtual void write_impl(unsigned verbosity, unsigned type, const std::string &str) override {
			std::ostream &out = getStream();
			using namespace std;
			if (verbosity == 0) {
				switch (type) {
				case Log::warning:
					out << termcolor::boldYellow; break;
				case Log::error:
					out << termcolor::boldRed; break;
				default:
					out << termcolor::boldCyan;
				}
			} else {
				switch (type) {
				case Log::warning:
					out << termcolor::yellow; break;
				case Log::error:
					out << termcolor::red; break;
				default:
					out << termcolor::reset;
				}
			}
			out << str;
			out << termcolor::reset;
			out << endl;
		}

	public:
		explicit inline ColoredStreamLogOutput(std::ostream *out_, bool mute_ = false) : StreamLogOutput(out_, mute_) { }
	};

	class FileLogOutput : public StreamLogOutput {
	private:
		std::ofstream m_out;

	public:
		explicit inline FileLogOutput(
			const std::string &fname_,
			std::ios_base::openmode mode_ = std::ios_base::trunc,
			bool mute_ = false
		) :
			StreamLogOutput(&m_out, mute_)
		{
			m_out.open(fname_, mode_);
		}
	};
	
	template <typename CharT>
	class basic_logstream : public std::basic_ostream<CharT> {
	private:
		unsigned m_verbosity;
		unsigned m_type;
		std::string m_source;
		std::basic_stringbuf<CharT> m_buf;
		bool m_write;

		basic_logstream(const basic_logstream &rhs) = delete;
		basic_logstream & operator=(const basic_logstream &rhs) = delete;

	public:
		// main ctor; sets type to Log::information and verbosity to 2
		basic_logstream(const std::string &source_) :
			std::basic_ostream<CharT>(&m_buf),
			m_verbosity(2),
			m_type(Log::information),
			m_source(source_),
			m_write(true)
		{ }

		// move ctor; takes over responsibility for writing to log
		basic_logstream(basic_logstream &&rhs) :
			m_verbosity(rhs.m_verbosity),
			m_type(rhs.m_type),
			m_source(rhs.m_source),
			m_buf(rhs.m_buf.str()),
			m_write(rhs.m_write)
		{
			rhs.m_write = false;
		}

		// set type to Log::warning and verbosity to 1
		basic_logstream & warning() {
			m_type = Log::warning;
			m_verbosity = 1;
			return *this;
		}

		// set type to Log::error and verbosity to 0
		basic_logstream & error() {
			m_type = Log::error;
			m_verbosity = 0;
			return *this;
		}

		// set verbosity
		basic_logstream & operator%(unsigned verbosity) {
			m_verbosity = verbosity;
			return *this;
		}

		~basic_logstream() {
			if (m_write) {
				Log::write(m_verbosity, m_type, m_source, m_buf.str());
			}
		}

	};
	
	using logstream = basic_logstream<char>;

	inline logstream log(const std::string &source) {
		return logstream(source);
	}

	inline logstream log() {
		return logstream("Global");
	}

}

#endif // AMBITION_LOG_HPP