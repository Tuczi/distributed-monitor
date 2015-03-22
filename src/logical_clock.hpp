#ifndef LOGIC_CLOCK_HPP_DEFINED
#define LOGIC_CLOCK_HPP_DEFINED

#include <cstdint>

namespace distributed_monitor {

/**
 * Logical clock
 * 
 * implementation of scalar logical clock
 */
template <typename t>
class logical_clock {
	private:
		/// Primitive representation of logical clock
		t value=0;
		
	public:
		typedef t primitve_type;
		
		inline void update() {
			value++;
		}
		
		inline void update(const logical_clock& clock) {
			if(*this<clock)
				value=clock.value;
		}
		
		inline bool operator < (const logical_clock& clock) const {
			return value < clock.value;
		}
		
		inline bool operator == (const logical_clock& clock) const {
			return value == clock.value;
		}
		
		inline bool operator > (const logical_clock& clock) const {
			return value > clock.value;
		}
		
		#ifdef DEBUG
		inline t get_value() {
			return value;
		}
		#endif
};

typedef logical_clock<uint32_t> logical_clock_uint;

}
#endif
