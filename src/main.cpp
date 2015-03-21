#include <cstdlib>
#include <mpi.h>
#include <iostream>

#include "program-monitor.hpp"

int application_thread(int& argc, char**& argv) {
	std::cout<<"Hello"<<std::endl;
	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
	
	MPI::Init(argc, argv);
	
	distributed_monitor::program_monitor monitor(100);
	monitor.run();
	
	int result = application_thread(argc, argv);
	//MPI::Finalize();
	return result;
}
