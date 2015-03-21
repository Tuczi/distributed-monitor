#ifndef LOGIC_CLOCK_HPP_DEFINED
#define LOGIC_CLOCK_HPP_DEFINED

#include <cstdint>

namespace distributed_monitor {

/**
 * Logical clock
 * 
 * implementation of scalar logical clock
 */
class logic_clock {
	private:
		/// Primitive representation of logic clock
		uint32_t value=0;
		
	public:
		inline void update() {
			value++;
		}
		
		inline void update(const logic_clock& clock) {
			value=clock.value;
		}
		
		inline bool operator < (const logic_clock& clock) {
			return value < clock.value;
		}
};

}
#endif
