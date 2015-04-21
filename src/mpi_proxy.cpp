#include "mpi_proxy.hpp"

namespace distributed_monitor {
  void mpi_proxy::receive_msg() {
    std::this_thread::yield();
    std::lock_guard<std::mutex> lk(data_map_mutex);
    for(auto& it: data_map) {
      auto& data = it.second;
      if(data.try_rcv(it.first)) {
				#ifdef DEBUG
				std::cout<<"mpi_proxy rcv success. tag: "<<it.first<<std::endl;
				#endif
        data.nonblocking_status = rcv_data::rcv_status::ready;
        data.cv.notify_one();
      }
    }
  }
  
  
	MPI::Status mpi_proxy::recv(void* data, const int count, const int tag, const MPI::Intracomm& comm) {
		#ifdef DEBUG
		std::cout<<comm.Get_rank()<<" want to rcv "<<std::endl;
		#endif
		std::unique_lock<std::mutex> global_add_lock(data_map_mutex);
		data_map.emplace(std::piecewise_construct,
			std::forward_as_tuple(tag),
			std::forward_as_tuple(data, count, (MPI::Intracomm&)comm));
		global_add_lock.unlock();
		
		std::mutex mutex;
		std::unique_lock<std::mutex> local_lock(mutex);
		
		data_map[tag].wait(local_lock);
		#ifdef DEBUG
		std::cout<<comm.Get_rank()<<" rcv continue "<<std::endl;
		#endif
		auto status = data_map[tag].status;
		
		std::unique_lock<std::mutex> global_rm_lock(data_map_mutex);
		data_map.erase(tag);
		
		return status;
	}
}
