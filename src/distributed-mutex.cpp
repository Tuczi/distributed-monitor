#include "distributed-mutex.hpp"
#include <unistd.h>
namespace distributed_monitor {
		
void distributed_mutex::request() {
	clock.update();
	request_ts = clock;
	broadcast(mpi_serial_t(mpi_serial_t::type_t::REQUEST, resource_id, request_ts));
	
	waiting = true;
}

bool distributed_mutex::can_enter() {
	return waiting && MPI::COMM_WORLD.Get_size()-1 == response_counter;
}

void distributed_mutex::response() {
	const auto& comm = MPI::COMM_WORLD;
	for(int i=0; i<comm.Get_size(); i++) {
		if(waiting_for_respose[i]) {
			waiting_for_respose[i]=false;
			
			request_ts = clock;
			send(mpi_serial_t(mpi_serial_t::type_t::RESPONSE, resource_id, request_ts), i);
		}
	}
	
	waiting = false;
}

void distributed_mutex::on_notify() {
	//TODO
}

void distributed_mutex::lock() {
	request();
	while(!can_enter());
}

void distributed_mutex::unlock() {
	response();
}

}
