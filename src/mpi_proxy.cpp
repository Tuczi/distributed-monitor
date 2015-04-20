#include "mpi_proxy.hpp"

namespace distributed_monitor {
  void mpi_proxy::receive_msg() {
    //std::this_thread::yield();
    std::lock_guard<std::mutex> lk(data_map_mutex);
    for(auto& it: data_map) {
      auto& data = it.second;
      if(data.try_rcv(it.first)) {
				#ifdef DEBUG
				std::cout<<"mpi_proxy rcv success. tag: "<<it.first<<std::endl;
				#endif
        data.ready = true;
        data.cv.notify_one();
      }
    }
  }
}
