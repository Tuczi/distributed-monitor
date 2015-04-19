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
      std::condition_variable cv;
      
      void* data;
      int count;
      MPI::Status status;
      
      bool ready;
      MPI::Intracomm& comm;
      
      rcv_data(): comm(MPI::COMM_WORLD) {}
      rcv_data(void* data, int count, MPI::Intracomm& comm): data(data), count(count), ready(false), comm(comm) { }
      //rcv_data(rcv_data&& o): cv(o.cv), data(std::move(o.data)), count(o.count), status(std::move(o.status)), ready(o.ready), comm(std::move(o.comm)) { }
      
      inline void wait(std::unique_lock<std::mutex>& lk) {
        cv.wait(lk, [this]()->bool{return ready;});
      }
      
      inline MPI::Request check_tag(int tag) {
        return comm.Irecv(data, count, MPI_BYTE, MPI_ANY_SOURCE, tag);
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
		}
    
		template <typename t> 
		void broadcast(const t& data, const int tag, const MPI::Intracomm& comm) {
			const int size = comm.Get_size(), rank = comm.Get_rank();
			
			for(int i=0; i<rank; i++)
				send(data, i, tag, comm);
			for(int i=rank+1; i<size; i++)
				send(data, i, tag, comm);
		}
    
    MPI::Status recv(void* data, const int count, const int tag, const MPI::Intracomm& comm) {
      std::unique_lock<std::mutex> global_lock(data_map_mutex);
      data_map.emplace(std::piecewise_construct,
        std::forward_as_tuple(tag),
        std::forward_as_tuple(data, count, (MPI::Intracomm&)comm));
      global_lock.unlock();
      
      std::mutex mutex;
      std::unique_lock<std::mutex> local_lock(mutex);
      
      data_map[tag].wait(local_lock);
      
      return data_map[tag].status;
    }
    
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
