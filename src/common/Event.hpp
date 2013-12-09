#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>

namespace ambition {
	template <class T>
	class Delegate {
	public:
		Delegate() {}
		virtual ~Delegate() {}
		virtual void fire(T*) = 0;
	};

	template <class T>
	class Event {
		std::vector<Delegate<T> *> observers;
	public:
		Event() {}
		virtual ~Event() {}
		void attach(Delegate<T> &d) { observers.push_back(&d); }
		void notify() {
			
			for(auto d : observers) {
				// FIXME - WHAT THE HELL IS THIS DANGEROUS-LOOKING CAST??? -Ben
				d->fire(static_cast<T*>(this));
			}
		}
	};
}

#endif