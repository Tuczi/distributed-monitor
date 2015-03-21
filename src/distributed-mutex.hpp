#ifndef DISTRIBUTED_MUTEX_HPP_DEFINED
#define DISTRIBUTED_MUTEX_HPP_DEFINED

#include <cstdint>
#include <mpi.h>

#include "logic-clock.hpp"

namespace distributed_monitor {

// http://en.wikipedia.org/wiki/Ricart%E2%80%93Agrawala_algorithm
class distributed_mutex {
	struct request_t {
		const uint8_t type=1;
		uint32_t resource_id;
		logic_clock ts;
		
		request_t(uint32_t resource_id, logic_clock ts): resource_id(resource_id), ts(ts) { }
	};
	struct responce_t {
		const uint8_t type=2;
		uint32_t resource_id;
		logic_clock ts;
		
		responce_t(uint32_t resource_id, logic_clock ts): resource_id(resource_id), ts(ts) { }
	};
	
	public:
		uint32_t resource_id;//TODO move to private
	private:
		logic_clock clock;
		logic_clock request_ts;
		bool waiting;
		uint32_t response_counter;
		int tag;
		
		template <typename t> 
		void broadcast(const t& data) {
			const auto& comm = MPI::COMM_WORLD;
			const int size = comm.Get_size(), rank = comm.Get_rank();
			
			for(int i=0; i<rank; i++)
				comm.Send(&data, sizeof(t), MPI_BYTE, i, tag);
			for(int i=rank+1; i<size; i++)
				comm.Send(&data, sizeof(t), MPI_BYTE, i, tag);
		}
	public:
		distributed_mutex(uint32_t resource_id): resource_id(resource_id), waiting(false), response_counter(0) { }
		
		void request();//TODO return wakup_variable
		bool can_enter();
		void response();
		
		void on_notify();
};

}
#endif
