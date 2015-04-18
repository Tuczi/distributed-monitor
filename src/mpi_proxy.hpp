#ifndef MPI_PROXY_HPP_DEFINED
#define MPI_PROXY_HPP_DEFINED

#include <thread>
#include <mutex>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <mpi.h>

#include "distributed_mutex.hpp"

namespace distributed_monitor {

/**
 * MPI proxy
 * 
 * provides MPI_THREAD_MULTIPLE functionality (calls mpi send from multiple thread)
 * on MPI_THREAD_FUNNELED level
 * 
 * TODO
 */
class mpi_proxy {
	typedef void (*callback_t)();
	private:		
		/// thread of this monitor
		std::thread rcv_thread;
		
		/// thread of this monitor
		std::mutex callbacks_mutex;
		
		/// map <tag, callback>
		std::unordered_map<int, callback_t> callbacks;
		
		void receive_msg();
		void notify(int tag);
		
		//TODO move broadcasts
		
  public:
		mpi_proxy() { }
		~mpi_proxy() { 
			rcv_thread.join();
		}
		
		//TODO send
		
		/// run proxy thread
		void run();
};

}
#endif

