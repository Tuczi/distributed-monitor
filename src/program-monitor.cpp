#include "program-monitor.hpp"

namespace distributed_monitor {

void program_monitor::recive_msg() {
}

void program_monitor::notify(uint32_t resource_id) {
	m_mutex.lock();
	
	auto it = mutexes.find(resource_id);
	if(it == mutexes.end()) {
		//TODO error or response - create static mutex method 
	} else {
		it->second.on_notify();
	}
	
	m_mutex.unlock();
}

void program_monitor::add(distributed_mutex& mutex) {
	m_mutex.lock();
	
	mutexes.emplace(mutex.resource_id, mutex);
	
	m_mutex.unlock();
}

void program_monitor::remove(const distributed_mutex& mutex) {
	m_mutex.lock();
	
	mutexes.erase(mutex.resource_id);
	
	m_mutex.unlock();
}

void program_monitor::run() {
	m_thread = std::thread([&]()->void {
		while(true) {
			std::cout << "A\n";
		}
	});
}

}
