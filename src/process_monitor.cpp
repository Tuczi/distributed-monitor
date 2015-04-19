#include "process_monitor.hpp"

namespace distributed_monitor {

void process_monitor::receive_msg() {
	distributed_mutex::mpi_serial_t data;
	MPI::Status status = proxy.recv(&data, sizeof(data), tag, comm);

	std::lock_guard<std::mutex> lk(l_mutex);
	
	auto it = d_mutexes.find(data.resource_id);
	#ifdef DEBUG
	std::cout<<"RCV(T:"<<comm.Get_rank()<<", F:"<<status.Get_source()
		<<") TYPE: "<<data.type<<",TS: "<<data.ts.get_value()
		<<",CLK: "<<(it == d_mutexes.end() ? 0 : it->second.clock.get_value())
		<<",RQ_TS: "<<(it == d_mutexes.end() ? 0 : it->second.request_ts.get_value())
		<<std::endl;
	#endif
	switch(data.type) {
		case distributed_mutex::mpi_serial_t::type_t::REQUEST:
			if(it == d_mutexes.end()) {
				data.type = distributed_mutex::mpi_serial_t::type_t::RESPONSE;
				proxy.send(data, status.Get_source(), tag, comm);
			} else {
				it->second.clock.update(data.ts);
				if( it->second.has_priority(data.ts, status.Get_source()) )
					it->second.waiting_for_respose[status.Get_source()] = true;
				else {
					data.type=distributed_mutex::mpi_serial_t::type_t::RESPONSE;
					it->second.clock.update();
					data.ts=it->second.clock;
					proxy.send(data, status.Get_source(), tag, comm);
				}
			}
			break;
			
		case distributed_mutex::mpi_serial_t::type_t::RESPONSE:
			it->second.response_counter++;
			if(it->second.can_enter()) {
				notify(it->second);
			}
	}
}

void process_monitor::add(distributed_mutex& mutex) {
	std::lock_guard<std::mutex> guard(l_mutex);
	
	d_mutexes.emplace(mutex.resource_id, mutex);
	mutex.p_monitor=this;
	mutex.waiting_for_respose.resize(comm.Get_size());
}

void process_monitor::remove(const distributed_mutex& mutex) {
	std::lock_guard<std::mutex> guard(l_mutex);
	
	d_mutexes.erase(mutex.resource_id);
}

void process_monitor::notify(distributed_mutex& mutex) {
	mutex.on_notify();
}

void process_monitor::run() {
  proxy.run();
	l_thread = std::thread([&]()->void {
		while(true) {
			this->receive_msg();
		}
	});
}

}
