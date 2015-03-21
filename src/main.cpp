#include <cstdlib>
#include <mpi.h>
#include <iostream>

#include "program-monitor.hpp"

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
		distributed_monitor::distributed_mutex mutex(comm.Get_rank());
		monitor.add(mutex);
		std::cout<<"Registered"<<std::endl;
	
		mutex.request();
		std::cout<<"Request sent"<<std::endl;
		while(!mutex.can_enter());
		
		std::cout<<"In critical section"<<std::endl;
		
		mutex.response();
	}
	while(true);
	return EXIT_SUCCESS;
}
