#include "distributed_mutex.hpp"

namespace distributed_monitor {

distributed_mutex::~distributed_mutex() {
	/*const auto* tmp= p_monitor;
	if(tmp) {
		tmp->remove(*this);
	}*/ //TODO
}
		
void distributed_mutex::request() {
	std::lock_guard<std::mutex> guard(p_monitor->l_mutex);
	
	response_counter=1;
	clock.update();
	request_ts = clock;
	p_monitor->proxy.broadcast(mpi_serial_t(mpi_serial_t::type_t::REQUEST, resource_id, request_ts));
}

bool distributed_mutex::can_enter() const {
	return (uint32_t) p_monitor->comm.Get_size() == response_counter;
}

void distributed_mutex::response() {
	std::lock_guard<std::mutex> guard(p_monitor->l_mutex);
	
	clock.update();
	request_ts = clock;
	for(int i=0; i<p_monitor->comm.Get_size(); i++) {
		if(waiting_for_respose[i]) {
			waiting_for_respose[i] = false;
			
			p_monitor->proxy.send(mpi_serial_t(mpi_serial_t::type_t::RESPONSE, resource_id, request_ts), i);
		}
	}
	
	response_counter=0;
}

bool distributed_mutex::has_priority(const logical_clock_uint& data_ts, const int source) const {
	return response_counter && 
		( (request_ts < data_ts) || (request_ts == data_ts && p_monitor->comm.Get_rank() < source) );
}

void distributed_mutex::on_notify() {
	l_condition.notify_one();
}

void distributed_mutex::lock() {
	request();
	
	std::unique_lock<std::mutex> lk(l_mutex);
	l_condition.wait(lk, [this]()-> bool {
		return this->can_enter();
	});
}

void distributed_mutex::unlock() {
	response();
}

}
