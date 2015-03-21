#ifndef DISTRIBUTED_MUTEX_HPP_DEFINED
#define DISTRIBUTED_MUTEX_HPP_DEFINED

#include <cstdint>
#include <vector>
#include <condition_variable>
#include <mpi.h>

#include "program-monitor.hpp"
#include "logic-clock.hpp"

namespace distributed_monitor {
class program_monitor;//forward declaration

// http://en.wikipedia.org/wiki/Ricart%E2%80%93Agrawala_algorithm
class distributed_mutex {
	friend program_monitor;
	struct mpi_serial_t {
		enum type_t: uint8_t {
			REQUEST=1, RESPONSE
		};
		
		type_t type;
		uint32_t resource_id;
		logic_clock ts;
		
		mpi_serial_t() { }
		mpi_serial_t(type_t type, uint32_t resource_id, logic_clock ts): type(type), resource_id(resource_id), ts(ts) { }
	};
	
	private:
		uint32_t resource_id;
		program_monitor* p_monitor = nullptr;
		std::condition_variable condition;
		
		logic_clock clock;
		logic_clock request_ts;
		bool waiting=false;
		uint32_t response_counter=0;
		std::vector<bool> waiting_for_respose;
		
		void request();
		bool can_enter();
		void response();
		
		void on_notify();
		
	public:
		distributed_mutex(uint32_t resource_id): resource_id(resource_id) { }
		
		void lock();
		void unlock();
};

}
#endif
