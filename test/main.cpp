#include <mpi.h>
#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>

#include "../src/distributed_monitor.hpp"

static distributed_monitor::process_monitor monitor(100);

/**
 * Distributed monitor example.
 * A distributed buffer.
 */
template <typename value_t>
class simple_buffer_t {
  struct mpi_data_t {
    ///true if push othervise get
    bool is_push;
    value_t value;
    
    mpi_data_t(bool is_push, value_t value): is_push(is_push), value(value) { }
    mpi_data_t() { }
  };
	public:
    int internal_mpi_update_tag;
		int internal_mpi_ack_tag;
    const MPI::Intracomm& comm = MPI::COMM_WORLD;
		distributed_monitor::distributed_mutex d_mutex;
    
    std::thread update_thread;
    
    std::queue<value_t> data;
	
		simple_buffer_t(int tag, int tag2): internal_mpi_update_tag(tag), internal_mpi_ack_tag(tag2), d_mutex(tag) {
			monitor.add(d_mutex);
      
      update_thread = std::thread([this]()->void{
        while(true)
          this->run_update_thread();
      });
		}
    ~simple_buffer_t() {
     update_thread.join(); 
    }
		
		void push(const value_t& v) {
			std::lock_guard<distributed_monitor::distributed_mutex> guard(d_mutex);
      
      std::cout<<comm.Get_rank()<<" push - in critical section"<<std::endl;
      
			data.push(v);
      
      send_update(true, v);
      
      std::cout<<comm.Get_rank()<<" push - exiting critical section"<<std::endl;
		}
		
		value_t get() {
			std::lock_guard<distributed_monitor::distributed_mutex> guard(d_mutex);
      
      std::cout<<comm.Get_rank()<<" get - in critical section"<<std::endl;
      
      value_t result = std::move(data.front());
      data.pop();
			
      send_update(false, result);
      
      std::cout<<comm.Get_rank()<<" get - exiting critical section with value "<<result<<std::endl;
      
      return result;
		}
    
    /**
     * Run update in single thread.
     * When new message is rcv then mutex is lock in other process
     * (so other thread should not modify data locally in the same time!).
     * Send Ack after update.
     * 
     * MPI guarantee fifo link order only in the same communicator.
     */
    void run_update_thread() {
      mpi_data_t mpi_data;
      MPI::Status status = monitor.proxy.recv(&mpi_data, sizeof(mpi_data), internal_mpi_update_tag, comm);
      
      std::cout<<comm.Get_rank()<<" run_update_thread rcv from"<< status.Get_source() <<std::endl;
      ///update data
      if(mpi_data.is_push)
        data.push(mpi_data.value);
      else
        data.pop();
        
      uint8_t ack_data;
      monitor.proxy.send(ack_data, status.Get_source(), internal_mpi_ack_tag, comm);
    }
    
    void send_update(const bool isPush, const value_t& v) {
      mpi_data_t mpi_data(isPush, v);
      MPI::Status status;
      
      monitor.proxy.broadcast(mpi_data, internal_mpi_update_tag, comm);
      
      const int size = comm.Get_size();
      uint8_t ack_data;
      ///ack counter
      for(int i=1; i<comm.Get_size(); i++)
        status = monitor.proxy.recv(&ack_data, sizeof(ack_data), internal_mpi_ack_tag, comm);
    }
};

int application_thread(int&, char**&);

int main(int argc, char** argv) {
	
	const auto mpi_expected_level = MPI_THREAD_FUNNELED; //MPI_THREAD_MULTIPLE;
	const auto mpi_provided_level = MPI::Init_thread(argc, argv, mpi_expected_level);
	if(mpi_provided_level < mpi_expected_level) {
		std::cout<<"expected thread level: "<<mpi_expected_level<<", provided: "<<mpi_provided_level<<std::endl;
		
		MPI::Finalize();
		return EXIT_FAILURE;
	}
	
	monitor.run();
	int result = application_thread(argc, argv);
	
	MPI::Finalize();
	return result;
}

int application_thread(int& argc, char**& argv) {
	const auto& comm = MPI::COMM_WORLD;
	std::cout<<"Hello"<<std::endl;
	
	simple_buffer_t<int> buffer(11, 12);
	comm.Barrier();
	std::cout<<"Hello2"<<std::endl;
	
	if(comm.Get_rank()==1)
		buffer.push(comm.Get_rank());
		
	//std::this_thread::sleep_for(std::chrono::seconds(1));
  if(comm.Get_rank()==2) 
		buffer.get();
  
	std::cout<<"Bye"<<std::endl;

	while(true)
		std::this_thread::sleep_for(std::chrono::seconds(10));
	
	return EXIT_SUCCESS;
}
