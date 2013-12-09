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
		static unsigned s_next_key;
		std::map<unsigned, std::function<void(EventArgT)> > m_observers;
		std::mutex m_lock;
		std::condition_variable m_cond;

	public:
		Event() {}

		unsigned attach(std::function<void(EventArgT)> func) {
			std::lock_guard<std::mutex> guard(m_lock);
			m_observers[s_next_key++] = func;
			return 0;
		}

		bool detach(unsigned key) {
			// TODO detach from within callback?
			std::lock_guard<std::mutex> guard(m_lock);
			return m_observers.erase(key);
		}

		void notify(EventArgT e) {
			std::lock_guard<std::mutex> guard(m_lock);
			for(auto pair : m_observers) {
				pair.second(e);
			}
			m_cond.notify_all();
		}

		void wait() {
			// TODO this doesnt suppress spurious wakeup
			std::unique_lock<std::mutex> guard(m_lock);
			m_cond.wait(guard);
		}
	};

	template <typename EventArgT>
	unsigned Event<EventArgT>::s_next_key = 0;

}

#endif