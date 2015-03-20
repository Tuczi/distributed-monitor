#ifndef DISTRIBUTED_MUTEX_HPP_DEFINED
#define DISTRIBUTED_MUTEX_HPP_DEFINED

#include <cstdint>

#include "logic-clock.hpp"

namespace distributed_monitor {

// http://en.wikipedia.org/wiki/Ricart%E2%80%93Agrawala_algorithm
class distributed_mutex {
	struct request_t {
		uint32_t resource_id;
		logic_clock ts;
		
		request_t(uint32_t resource_id, logic_clock ts): resource_id(resource_id), ts(ts) { }
	};
	struct responce_t {
		uint32_t resource_id;
		logic_clock ts;
		
		responce_t(uint32_t resource_id, logic_clock ts): resource_id(resource_id), ts(ts) { }
	};
	
	private:
		uint32_t resource_id;
		logic_clock clock;
		logic_clock request_ts;
		bool waiting;
		uint32_t response_counter;
		
	public:
		distributed_mutex(uint32_t resource_id): resource_id(resource_id), waiting(false), response_counter(0) { }
		
		void request();//TODO return wakup_variable
		bool can_enter();
		void response();
		
		void on_notify();
};

}
#endif
