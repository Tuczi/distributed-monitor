#ifndef DISTRIBUTED_MUTEX_HPP_DEFINED
#define DISTRIBUTED_MUTEX_HPP_DEFINED

#include <cstdint>
#include <vector>
#include <mpi.h>

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
	
	public:
		uint32_t resource_id;//TODO move to private
	private:
		logic_clock clock;
		logic_clock request_ts;
		bool waiting=false;
		uint32_t response_counter=0;
		std::vector<bool> waiting_for_respose;
		int tag=-1;
		
		template <typename t>
		void send(const t& data, int to) {
			const auto& comm = MPI::COMM_WORLD;
			comm.Send(&data, sizeof(t), MPI_BYTE, to, tag);
		}
		
		template <typename t> 
		void broadcast(const t& data) {
			const auto& comm = MPI::COMM_WORLD;
			const int size = comm.Get_size(), rank = comm.Get_rank();
			
			for(int i=0; i<rank; i++)
				send(data, i);
			for(int i=rank+1; i<size; i++)
				send(data, i);
		}
	public:
		distributed_mutex(uint32_t resource_id): resource_id(resource_id), waiting_for_respose(MPI::COMM_WORLD.Get_size()) { }
		
		void request();//TODO return wakup_variable
		bool can_enter();
		void response();
		
		void on_notify();
};

}
#endif
