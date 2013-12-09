#ifndef EVENT_HPP
#define EVENT_HPP

#include <map>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace ambition {

	template <class EventArgT>
	class Event {
	private:
		unsigned m_next_key = 0;
		unsigned m_count = 0;
		std::map<unsigned, std::function<void(EventArgT)> > m_observers;
		std::mutex m_lock;
		std::condition_variable m_cond;

	public:
		Event() {}

		unsigned attach(std::function<void(EventArgT)> func) {
			std::lock_guard<std::mutex> guard(m_lock);
			m_observers[m_next_key++] = func;
			return 0;
		}

		bool detach(unsigned key) {
			// TODO detach from within callback?
			std::lock_guard<std::mutex> guard(m_lock);
			return m_observers.erase(key);
		}

		void notify(EventArgT e) {
			std::lock_guard<std::mutex> guard(m_lock);
			m_count++;
			for(auto pair : m_observers) {
				pair.second(e);
			}
			m_cond.notify_all();
		}

		/* Returns true if the event was fired. */
		bool wait() {
			std::unique_lock<std::mutex> guard(m_lock);
			// record the notify count at start of waiting
			unsigned count0 = m_count;
			m_cond.wait(guard);
			// if the notify count changed, the event was triggered
			return m_count != count0;
		}
	};

}

#endif