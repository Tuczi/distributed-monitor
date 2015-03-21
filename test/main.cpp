#include <cstdlib>
#include <mpi.h>
#include <iostream>
#include <unistd.h>
#include <mutex>

#include "../src/distributed_monitor.hpp"

static distributed_monitor::process_monitor monitor(100);

class simple_monitor_t {
	public:
		distributed_monitor::distributed_mutex d_mutex;
	
		simple_monitor_t(): d_mutex(11) {
			monitor.add(d_mutex);
		}
		
		void doSth() {
			std::lock_guard<distributed_monitor::distributed_mutex> guard(d_mutex);
			
			const auto& comm = MPI::COMM_WORLD;
			std::cout<<comm.Get_rank()<<" doSth - in critical section"<<std::endl;
			sleep(5);	
			std::cout<<comm.Get_rank()<<" doSth - exiting critical section"<<std::endl;
			
		}
		
		void doSth2() {
			std::lock_guard<distributed_monitor::distributed_mutex> guard(d_mutex);
			
			const auto& comm = MPI::COMM_WORLD;
			std::cout<<comm.Get_rank()<<" doSth2 - in critical section"<<std::endl;
			sleep(5);	
			std::cout<<comm.Get_rank()<<" doSth2 - exiting critical section"<<std::endl;
		}
};

int application_thread(int&, char**&);

int main(int argc, char** argv) {
	
	MPI::Init(argc, argv);
	
	monitor.run();
	int result = application_thread(argc, argv);
	
	MPI::Finalize();
	return result;
}

int application_thread(int& argc, char**& argv) {
	const auto& comm = MPI::COMM_WORLD;
	std::cout<<"Hello"<<std::endl;
	
	if(comm.Get_rank()<2) {
		simple_monitor_t simple_monitor; 
		sleep(1);
		
		simple_monitor.doSth();
		simple_monitor.doSth2();
	}

	while(true);
	return EXIT_SUCCESS;
}
