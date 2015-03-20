#include "distributed-mutex.hpp"

namespace distributed_monitor {
		
void distributed_mutex::request() {
}

bool distributed_mutex::can_enter() {
	return false;
}

void distributed_mutex::response() {
}

void distributed_mutex::on_notify() {
}

}
