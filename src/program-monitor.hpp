#ifndef PROGRAM_MONITOR_HPP_DEFINED
#define PROGRAM_MONITOR_HPP_DEFINED

#include <thread>
#include <mutex>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <mpi.h>

#include "distributed-mutex.hpp"

namespace distributed_monitor {
	
class program_monitor {
	private:
		int tag;
		std::thread m_thread;
		std::mutex m_mutex;
		std::unordered_map<uint32_t, distributed_mutex&> mutexes;
		
		void receive_msg();
		inline void notify(distributed_mutex& mutex) {
			mutex.on_notify();
		}
		
	public:
		program_monitor(int tag): tag(tag) { }
		~program_monitor() { 
			m_thread.join();
		}
		
		void add(distributed_mutex&);
		void remove(const distributed_mutex&);
		
		void run();
};

}
#endif
