#ifndef LOGIC_CLOCK_HPP_DEFINED
#define LOGIC_CLOCK_HPP_DEFINED

#include <cstdint>

namespace distributed_monitor {

/**
 * Logical clock
 * 
 * implementation of scalar logical clock
 */
class logical_clock {
	private:
		/// Primitive representation of logical clock
		uint32_t value=0;
		
	public:
		inline void update() {
			value++;
		}
		
		inline void update(const logical_clock& clock) {
			value=clock.value;
		}
		
		inline bool operator < (const logical_clock& clock) {
			return value < clock.value;
		}
};

}
#endif
