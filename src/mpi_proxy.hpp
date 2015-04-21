#ifndef MPI_PROXY_HPP_DEFINED
#define MPI_PROXY_HPP_DEFINED

#include <thread>
#include <mutex>
#include <cstdint>
#include <condition_variable>
#include <unordered_map>
#include <iostream>
#include <mpi.h>

namespace distributed_monitor {

/**
 * MPI proxy
 * 
 * provides MPI_THREAD_MULTIPLE functionality (calls mpi send and recv from multiple thread)
 * on MPI_THREAD_FUNNELED level
 * 
 * this solution should be redundant in Open MPI 1.9 (current version is 1.8.4), because
 * MPI_THREAD_MULTIPLE should be fully support in that version (no is only "weakly tested")
 */
class mpi_proxy {
  public:
    struct rcv_data {
			enum rcv_status: uint_fast8_t {
				/// order of this values is important!
				ready = 0, initiated, not_initiated
			};
			
      std::condition_variable cv;
      
      void* data;
      int count;
      MPI::Status status;
      MPI::Request request;
      MPI::Intracomm& comm;
      
      rcv_status nonblocking_status;
      
      rcv_data(): comm(MPI::COMM_WORLD) {}
      rcv_data(void* data, int count, MPI::Intracomm& comm): data(data), count(count), comm(comm), nonblocking_status(rcv_status::not_initiated) { }
      
      inline void wait(std::unique_lock<std::mutex>& lk) {
        cv.wait(lk, [this]()->bool{return nonblocking_status==rcv_status::ready;});
      }
      
      inline bool try_rcv(int tag) {
				/// it was rcv previously
				if(nonblocking_status == rcv_status::ready)
					return false;
				
				///Call MPI Irecv once
				if(nonblocking_status > rcv_status::initiated) {
					request = comm.Irecv(data, count, MPI_BYTE, MPI_ANY_SOURCE, tag);
					nonblocking_status = rcv_status::initiated;
				}
				
				return request.Test(status);		
      }
      
    };
	private:		
    /// local thread
    std::thread l_thread;
		/// data_map mutex
		std::mutex data_map_mutex;
		
		/// map <tag, rcv_data>
		std::unordered_map<int, rcv_data> data_map;
    
    void receive_msg();
		
  public:
    mpi_proxy() { }
		~mpi_proxy() { }
    
    template <typename t>
		void send(const t& data, const int to, const int tag, const MPI::Intracomm& comm) {
			comm.Send(&data, sizeof(t), MPI_BYTE, to, tag);
			#ifdef DEBUG
			std::cout<<comm.Get_rank()<<" send to "<<to<<std::endl;
			#endif
		}
    
		template <typename t> 
		void broadcast(const t& data, const int tag, const MPI::Intracomm& comm) {
			const int size = comm.Get_size(), rank = comm.Get_rank();
			
			for(int i=0; i<rank; i++)
				send(data, i, tag, comm);
			for(int i=rank+1; i<size; i++)
				send(data, i, tag, comm);
		}
		
		inline void barrier(const MPI::Intracomm& comm) {
			comm.Barrier();
		}
    
    MPI::Status recv(void* data, const int count, const int tag, const MPI::Intracomm& comm);
    
    inline void run() {
      l_thread = std::thread([&]()->void {
        while(true) {
          this->receive_msg();
        }
      });
    }
};

}
#endif
