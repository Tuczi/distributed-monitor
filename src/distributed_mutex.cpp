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
	
	waiting = true;
	response_counter = 0;
	clock.update();
	request_ts = clock;
	p_monitor->broadcast(mpi_serial_t(mpi_serial_t::type_t::REQUEST, resource_id, request_ts));
}

bool distributed_mutex::can_enter() {
	return waiting && (uint32_t) p_monitor->comm.Get_size()-1 == response_counter;
}

void distributed_mutex::response() {
	std::lock_guard<std::mutex> guard(p_monitor->l_mutex);
	
	for(int i=0; i<p_monitor->comm.Get_size(); i++) {
		if(waiting_for_respose[i]) {
			waiting_for_respose[i] = false;
			
			request_ts = clock;
			p_monitor->send(mpi_serial_t(mpi_serial_t::type_t::RESPONSE, resource_id, request_ts), i);
		}
	}
	
	waiting = false;
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
