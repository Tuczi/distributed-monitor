#include <cstdlib>
#include <mpi.h>
#include <iostream>

#include "distributed-monitor.hpp"

int application_thread(int&, char**&);

static distributed_monitor::program_monitor monitor(100);

int main(int argc, char** argv) {
	
	MPI::Init(argc, argv);
	
	monitor.run();
	
	int result = application_thread(argc, argv);
	//MPI::Finalize();
	return result;
}

int application_thread(int& argc, char**& argv) {
	const auto& comm = MPI::COMM_WORLD;
	std::cout<<"Hello"<<std::endl;
	
	
	if(comm.Get_rank()<2) {
		distributed_monitor::distributed_mutex mutex(comm.Get_rank());//TODO only moniotr can create mutex
		monitor.add(mutex);
		std::cout<<"Registered"<<std::endl;
	
		mutex.lock();
		
		std::cout<<"In critical section"<<std::endl;
		
		mutex.unlock();
	}
	while(true);
	return EXIT_SUCCESS;
}
