#include "mpi_proxy.hpp"

namespace distributed_monitor {
  void mpi_proxy::receive_msg() {
    std::this_thread::yield();
    std::lock_guard(data_map_mutex) lk;
    for(auto& it: data_map) {
      auto request = it.check_tag(tag);
      if(request.Test(it.status)) {
        it.rady = true;
        it.notify();
      }
    }
}
