#ifndef PROCESS_MONITOR_HPP_DEFINED
#define PROCESS_MONITOR_HPP_DEFINED

#include <thread>
#include <mutex>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <mpi.h>

#include "mpi_proxy.hpp"
#include "distributed_mutex.hpp"

namespace distributed_monitor {
class distributed_mutex;

/**
 * Process monitor
 * 
 * runs in single std::thread
 * it provides communication via Open MPI (mpi_proxy class) and notify distributed_mutexes
 */
class process_monitor {
	friend distributed_mutex;
	private:
		const int tag;
		const MPI::Intracomm& comm;
		
		/// thread of this monitor
		std::thread l_thread;
		/// mutex to block data structures
		std::mutex l_mutex;
		/// map <resource_id, distributed_mutex>
		std::unordered_map<uint32_t, distributed_mutex&> d_mutexes;
		
		void receive_msg();
		void notify(distributed_mutex&);
		
	public:
    /// communication proxy
    mpi_proxy proxy;
    
		process_monitor(int tag, const MPI::Intracomm& comm=MPI::COMM_WORLD): tag(tag), comm(comm) { std::cout<<"A"<<std::endl; }
		~process_monitor() { 
			l_thread.join();
		}
		
		void add(distributed_mutex&);
		void remove(const distributed_mutex&);
		
		/// run process monitor thread
		void run();
};

}
#endif
