#ifndef AMBITION_CONCURRENT_HPP
#define AMBITION_CONCURRENT_HPP

#include <cassert>
#include <stdexcept>
#include <map>
#include <vector>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "Ambition.hpp"
#include "Log.hpp"

namespace ambition {

	class interruption : public std::runtime_error {
	public:
		interruption() : runtime_error("condition variable wait interrupted") {};
		// virtual const char * what() const override {
		// 	return "";
		// }
	};

	// High-level mechanism for providing interruption of condition variable waiting.
	// I dont know how well this actually performs, but it seems to work at least.
	// Only threads that are waiting using this class can be interrupted using this class.
	class InterruptManager {
	private:
		struct thread_data_t {
			std::condition_variable *condition;
			std::mutex *mutex;
			bool interrupt;
		};

		static std::mutex m_mutex;
		static std::map<std::thread::id, thread_data_t> m_thread_data;

	public:
		// wait on a condition variable.
		// lock should already be locked.
		static void wait(std::condition_variable &cond, std::unique_lock<std::mutex> &lock);

		// interrupt a thread waiting on a condition variable.
		// if thread is not waiting, it will be interrupted when next it does.
		static void interrupt(const std::thread::id &id);

		// interrupt all threads waiting on a condition variable.
		// the mutex this condition variable is waiting with is assumed to be locked already.
		static void interrupt(std::condition_variable &cond);

	};

	template <class EventArgT>
	class Event : private Uncopyable {
	public:
		// return true to detach
		using observer_t = std::function<bool(const EventArgT &)>;

	private:
		unsigned m_next_key = 0;
		unsigned m_count = 0;
		unsigned m_waiters = 0;
		std::map<unsigned, observer_t> m_observers;
		std::mutex m_mutex;
		std::condition_variable m_cond;

		class waiter_guard {
		private:
			unsigned *m_waiters;

		public:
			inline waiter_guard(unsigned &waiters_) : m_waiters(&waiters_) {
				(*m_waiters)++;
			}

			inline ~waiter_guard() {
				(*m_waiters)--;
			}
		};

	public:
		inline Event() {}

		inline unsigned attach(const observer_t &func) {
			std::lock_guard<std::mutex> lock(m_mutex);
			unsigned key = m_next_key++;
			m_observers[key] = func;
			return key;
		}

		inline bool detach(unsigned key) {
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_observers.erase(key);
		}

		inline void notify(const EventArgT &e) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_count++;
			if (!m_observers.empty()) {
				std::vector<unsigned> detach_keys;
				for (auto pair : m_observers) {
					if (pair.second(e)) {
						detach_keys.push_back(pair.first);
					}
				}
				for (auto key : detach_keys) {
					m_observers.erase(key);
				}
			}
			m_cond.notify_all();
		}

		// returns true if the event was fired
		inline bool wait() {
			std::unique_lock<std::mutex> lock(m_mutex);
			waiter_guard waiter(m_waiters);
			// record the notify count at start of waiting
			unsigned count0 = m_count;
			// if this thread was interrupted while waiting, this will throw
			InterruptManager::wait(m_cond, lock);
			// if the notify count changed, the event was triggered
			return m_count != count0;
		}

		// TODO timed wait etc

		inline ~Event() {
			// interrupt all waiting threads, then wait for them to unlock the mutex
			auto time0 = std::chrono::steady_clock::now();
			while (true) {
				std::this_thread::yield();
				std::lock_guard<std::mutex> lock(m_mutex);
				// test if we can go home yet
				if (m_waiters == 0) break;
				// interrupt any threads waiting on this event still
				InterruptManager::interrupt(m_cond);
				if (std::chrono::steady_clock::now() - time0 > std::chrono::milliseconds(100)) {
					// failed to finish within timeout
					log("Event").error() << "Destructor failed to finish within timeout";
					std::abort();
				}
			}
		}
	};

	// simple (and not quite finished) blocking queue
	template <typename T>
	class blocking_queue {
	private:
		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::deque<T> m_queue;

	public:
		inline blocking_queue() { }

		inline blocking_queue(const blocking_queue &other) {
			assert(false && "not implemented yet");
		}

		inline blocking_queue(blocking_queue &&other) {
			assert(false && "not implemented yet");
		}

		inline blocking_queue & operator=(const blocking_queue &other) {
			assert(false && "not implemented yet");
			return *this;
		}

		inline blocking_queue & operator=(blocking_queue &&other) {
			assert(false && "not implemented yet");
			return *this;
		}

		inline void push(T const& value) {
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				m_queue.push_front(value);
			}
			m_condition.notify_one();
		}

		inline T pop() {
			std::unique_lock<std::mutex> lock(m_mutex);
			while (m_queue.empty()) {
				// if this thread is interrupted while waiting, this will throw
				InterruptManager::wait(m_condition, lock);
			}
			T rc(std::move(m_queue.back()));
			m_queue.pop_back();
			return rc;
		}

		inline bool empty() {
			std::unique_lock<std::mutex> lock(m_mutex);
			return m_queue.empty();
		}

	};

	// mechanism for asynchronous execution of arbitrary tasks
	// yes i know std::async exists
	class AsyncExecutor {
	public:
		using task_t = std::function<void(void)>;

	private:
		static std::atomic<bool> m_started;
		static blocking_queue<task_t> m_backQueue;
		static blocking_queue<task_t> m_mainQueue;
		static std::thread m_thread;

	public:
		// start the background thread
		static void start();

		// stop the background thread. must be called before exit() to die nicely.
		// cannot be registered with atexit() due to MSVC stdlib bug
		// https://connect.microsoft.com/VisualStudio/feedback/details/747145/std-thread-join-hangs-if-called-after-main-exits-when-using-vs2012-rc
		static void stop();

		// add a background task
		static void enqueueBackground(const task_t &f);

		// add a task to the 'main' thread
		static void enqueueMain(const task_t &f);

		// execute tasks on the 'main' thread up to some time limit
		static void executeMain(double dt = 0.001);
	};

}

#endif