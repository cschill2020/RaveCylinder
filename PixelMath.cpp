#include "PixelMath.h"

namespace ravecylinder {

uint32_t get_millisecond_timer() {
  auto duration = clock_type::now().time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return static_cast<uint32_t>(millis);
}

long long GetTimeMicros(void) {
  auto duration = clock_type::now().time_since_epoch();
  auto micros =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  return static_cast<long long>(micros);
}

} // namespace ravecylinder