// Taken from FastLED.  

#ifndef __PIXELMATH_H
#define __PIXELMATH_H

#include <stdint.h>

namespace ravecylinder {

// Saturate at 0x00
inline uint8_t subtractColor(uint8_t i, uint8_t j) {
  int res = i - j;
  return (res < 0) ? 0x00 : res;
}

// Saturate at 0xFF
inline uint8_t addColor(uint8_t i, uint8_t j) {
  int res = i + j;
  return (res > 0xFF) ? 0xFF : res;
}

// Saturate at 0xFF
inline uint8_t multiplyColor(uint8_t i, uint8_t j) {
  int res = i * j;
  return (res > 0xFF) ? 0xFF : res;
}

} // namespace ravecylinder

#endif