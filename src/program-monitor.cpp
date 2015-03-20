#include "program-monitor.hpp"

namespace distributed_monitor {

void program_monitor::recive_msg() {
}

void program_monitor::notify(int resource_id) {
}

void program_monitor::add(const distributed_mutex&) {
}

void program_monitor::remove(const distributed_mutex&) {
}

void program_monitor::run() {
	monitor_thread = std::thread([&]()->void {
		while(true) {
			std::cout << "A\n";
		}
	});
}

}
