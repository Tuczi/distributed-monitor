#ifndef PROGRAM_MONITOR_HPP_DEFINED
#define PROGRAM_MONITOR_HPP_DEFINED

#include <thread>
#include <iostream>

#include "distributed-mutex.hpp"

namespace distributed_monitor {
	
class program_monitor {
	private:
		int tag;
		std::thread monitor_thread;
		
		void recive_msg();
		void notify(int resource_id);
		
	public:
		program_monitor(int tag): tag(tag) { }
		~program_monitor() { 
			monitor_thread.join();
		}
		
		void add(const distributed_mutex&);
		void remove(const distributed_mutex&);
		
		void run();
};

}
#endif
