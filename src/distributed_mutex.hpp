#ifndef DISTRIBUTED_MUTEX_HPP_DEFINED
#define DISTRIBUTED_MUTEX_HPP_DEFINED

#include <cstdint>
#include <vector>
#include <condition_variable>
#include <mpi.h>

#include "process_monitor.hpp"
#include "logical_clock.hpp"

namespace distributed_monitor {
class process_monitor;//forward declaration

/**
 * Distributed mutex
 * 
 * implementation of the Ricart-Agrawala algorithm for mutual exclusion on a distributed system
 * @see http://en.wikipedia.org/wiki/Ricart%E2%80%93Agrawala_algorithm
 */
class distributed_mutex {
	friend process_monitor;
	/**
	 * Structure for Open MPI message serialization
	 */
	struct mpi_serial_t {
		enum type_t: uint8_t {
			REQUEST=1, RESPONSE
		};
		
		type_t type;
		uint32_t resource_id;
		/// Timestamp
		logical_clock_uint ts;
		
		/// Constructor only for receiving messages
		mpi_serial_t() { }
		mpi_serial_t(type_t type, uint32_t resource_id, logical_clock_uint ts): type(type), resource_id(resource_id), ts(ts) { }
	};
	
	private:
		uint32_t resource_id;
		process_monitor* p_monitor = nullptr;
		
		/// mutex to wake up thread
		std::mutex l_mutex;
		/// condition variable to wake up thread
		std::condition_variable l_condition;
		
		logical_clock_uint clock;
		/// timestamp of last request (send)
		logical_clock_uint request_ts;
		
		/// >0 if process want to enter.
		uint32_t response_counter=0;
		/**
		 * If value of i-th element is true then
		 * on unlock there is necessary to send response to i-th process
		 */
		std::vector<bool> waiting_for_respose;
		
		/// true if local_process has priority over source process in mutual exclusion
		bool has_priority(const logical_clock_uint& data_ts, const int source) const;
		
		void request();
		bool can_enter() const;
		void response();
		
		void on_notify();
		
	public:
		distributed_mutex(uint32_t resource_id): resource_id(resource_id) { }
		~distributed_mutex();
		
		void lock();
		void unlock();
};

}
#endif
