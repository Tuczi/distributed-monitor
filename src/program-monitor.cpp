#include "program-monitor.hpp"

namespace distributed_monitor {

void program_monitor::receive_msg() {
	const auto& comm = MPI::COMM_WORLD;
	
	distributed_mutex::mpi_serial_t data;
	MPI::Status status;
	comm.Recv(&data, sizeof(data), MPI_BYTE, MPI_ANY_SOURCE, tag, status);
	
	std::lock_guard<std::mutex> guard(l_mutex);
	
	auto it = d_mutexes.find(data.resource_id);
	switch(data.type) {
		case distributed_mutex::mpi_serial_t::type_t::REQUEST:
			
			if(it == d_mutexes.end()) {
				data.type = distributed_mutex::mpi_serial_t::type_t::RESPONSE;
				send(data, status.Get_source());
			} else {
				if(it->second.waiting && it->second.request_ts < data.ts)
					it->second.waiting_for_respose[status.Get_source()] = true;
				else {
					data.type=distributed_mutex::mpi_serial_t::type_t::RESPONSE;
					it->second.clock.update();
					data.ts=it->second.clock;
					send(data, status.Get_source());
				}
			}
			break;
			
		case distributed_mutex::mpi_serial_t::type_t::RESPONSE:
			it->second.response_counter++;
			if(it->second.response_counter == comm.Get_size()-1) {
				notify(it->second);
			}
	}
}

void program_monitor::add(distributed_mutex& mutex) {
	std::lock_guard<std::mutex> guard(l_mutex);
	
	d_mutexes.emplace(mutex.resource_id, mutex);
	mutex.p_monitor=this;
	mutex.waiting_for_respose.resize(comm.Get_size());
}

void program_monitor::remove(const distributed_mutex& mutex) {
	std::lock_guard<std::mutex> guard(l_mutex);
	
	d_mutexes.erase(mutex.resource_id);
}

void program_monitor::notify(distributed_mutex& mutex) {
	mutex.on_notify();
}

void program_monitor::run() {
	l_thread = std::thread([&]()->void {
		while(true) {
			this->receive_msg();
		}
	});
}

}
