#include <cstdlib>
#include <mpi.h>
#include <iostream>
#include <unistd.h>

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
		distributed_monitor::distributed_mutex mutex(11);//TODO only moniotr can create mutex
		monitor.add(mutex);
		std::cout<<"Registered"<<std::endl;
		sleep(1+comm.Get_rank());
				
		{//CRITICAL SECTION
			std::lock_guard<distributed_monitor::distributed_mutex> guard(mutex);
			std::cout<<comm.Get_rank()<<"In critical section"<<std::endl;
			sleep(5);	
		}
		
		std::cout<<comm.Get_rank()<<"NOT In critical section"<<std::endl;
	}
	while(true);
	return EXIT_SUCCESS;
}
