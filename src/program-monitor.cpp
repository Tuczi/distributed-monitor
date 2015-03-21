#include "program-monitor.hpp"

namespace distributed_monitor {

void program_monitor::receive_msg() {
	const auto& comm = MPI::COMM_WORLD;
	
	distributed_mutex::mpi_serial_t data;
	MPI::Status status;
	comm.Recv(&data, sizeof(data), MPI_BYTE, MPI_ANY_SOURCE, tag, status);
	
	m_mutex.lock();
	
	auto it = mutexes.find(data.resource_id);
	switch(data.type) {
		case distributed_mutex::mpi_serial_t::type_t::REQUEST:
			
			if(it == mutexes.end()) {
				//TODO response - create static mutex method 
				data.type=distributed_mutex::mpi_serial_t::type_t::RESPONSE;//TODO correct ts
				comm.Send(&data, sizeof(data), MPI_BYTE, status.Get_source(), tag);
				
			} else {
				if(it->second.waiting && it->second.request_ts < data.ts)
					it->second.waiting_for_respose[status.Get_source()] = true;
				else {
					data.type=distributed_mutex::mpi_serial_t::type_t::RESPONSE;//TODO correct ts
					comm.Send(&data, sizeof(data), MPI_BYTE, status.Get_source(), tag);
				}
			}
			break;
			
		case distributed_mutex::mpi_serial_t::type_t::RESPONSE:
			it->second.response_counter++;
			std::cout<<it->second.response_counter<<std::endl;
			if(it->second.response_counter == comm.Get_size()-1) {
				notify(it->second);
			}
	}

	m_mutex.unlock();
}

void program_monitor::add(distributed_mutex& mutex) {
	m_mutex.lock();
	
	mutexes.emplace(mutex.resource_id, mutex);
	mutex.tag=tag;
	
	m_mutex.unlock();
}

void program_monitor::remove(const distributed_mutex& mutex) {
	m_mutex.lock();
	
	mutexes.erase(mutex.resource_id);
	
	m_mutex.unlock();
}

void program_monitor::run() {
	m_thread = std::thread([&]()->void {
		while(true) {
			this->receive_msg();
		}
	});
}

}
