// Taken from FastLED.

#ifndef __PIXELMATH_H
#define __PIXELMATH_H

#include <chrono>
#include <iostream>
#include <stdint.h>

namespace ravecylinder {
using std::chrono::high_resolution_clock;
typedef uint16_t accum88;
typedef uint16_t fract16;

uint32_t get_millisecond_timer(); 

#define GET_MILLIS get_millisecond_timer

// Saturate at 0x00
static uint8_t subtractColor(uint8_t i, uint8_t j) {
  int res = i - j;
  return (res < 0) ? 0x00 : res;
}

// Saturate at 0xFF
static uint8_t addColor(uint8_t i, uint8_t j) {
  int res = i + j;
  return (res > 0xFF) ? 0xFF : res;
}

// Saturate at 0xFF
static uint8_t multiplyColor(uint8_t i, uint8_t j) {
  int res = i * j;
  return (res > 0xFF) ? 0xFF : res;
}

// Scale is unit of scale/256.
static uint8_t scale8(uint8_t i, uint8_t scale) {
  return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
}

static uint8_t scale8_video(uint8_t i, uint8_t scale) {
  return (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
}

static uint16_t scale16(uint16_t i, fract16 scale) {
    return ((uint32_t)(i) * (1+(uint32_t)(scale))) / 65536;
}

/// Scale three one-byte values by a fourth one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it compute r,g,b * (scale / 256)
///
/// @warning This function always modifies its arguments in place!
/// @param r first value to scale
/// @param g second value to scale
/// @param b third value to scale
/// @param scale scale factor, in n/256 units
static void nscale8x3(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t scale) {
  uint16_t scale_fixed = scale + 1;
  r = (((uint16_t)r) * scale_fixed) >> 8;
  g = (((uint16_t)g) * scale_fixed) >> 8;
  b = (((uint16_t)b) * scale_fixed) >> 8;
}

/// Scale three one-byte values by a fourth one, which is treated as
/// the numerator of a fraction whose demominator is 256.
///
/// In other words, it computes r,g,b * (scale / 256), ensuring
/// that non-zero values passed in remain non-zero, no matter how low the scale
/// argument.
///
/// @warning This function always modifies its arguments in place!
/// @param r first value to scale
/// @param g second value to scale
/// @param b third value to scale
/// @param scale scale factor, in n/256 units
static void nscale8x3_video(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t scale) {
  uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
  r = (r == 0) ? 0 : (((int)r * (int)(scale)) >> 8) + nonzeroscale;
  g = (g == 0) ? 0 : (((int)g * (int)(scale)) >> 8) + nonzeroscale;
  b = (b == 0) ? 0 : (((int)b * (int)(scale)) >> 8) + nonzeroscale;
}

/// Blend a variable proportion (0-255) of one byte to another.
/// @param a the starting byte value
/// @param b the byte value to blend toward
/// @param amountOfB the proportion (0-255) of b to blend
/// @returns a byte value between a and b, inclusive
static uint8_t blend8(uint8_t a, uint8_t b, uint8_t amountOfB) {

  // The BLEND_FIXED formula is
  //
  //   result = (  A*(amountOfA) + B*(amountOfB)              )/ 256
  //
  // …where amountOfA = 255-amountOfB.
  //
  // This formula will never return 255, which is why the BLEND_FIXED +
  // SCALE8_FIXED version is
  //
  //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
  //
  // We can rearrange this formula for some great optimisations.
  //
  //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
  //          = (  A*(255-amountOfB) + A + B*(amountOfB) + B  ) / 256
  //          = (  A*(256-amountOfB) + B*(amountOfB) + B      ) / 256
  //          = (  A*256 + B + B*(amountOfB) - A*(amountOfB)  ) / 256  // this
  //          is the version used in SCALE8_FIXED AVR below = (  A*256 + B +
  //          (B-A)*(amountOfB)              ) / 256  // this is the version
  //          used in SCALE8_FIXED C below

  uint16_t partial;
  uint8_t result;
  partial = (a << 8) | b; // A*256 + B
  // on many platforms this compiles to a single multiply of (B-A) * amountOfB
  partial += (b * amountOfB);
  partial -= (a * amountOfB);
  result = partial >> 8;

  return result;
}

/// Map from one full-range 8-bit value into a narrower
/// range of 8-bit values, possibly a range of hues.
///
/// E.g. map `myValue` into a hue in the range blue..purple..pink..red
///   @code
///   hue = map8( myValue, HUE_BLUE, HUE_RED);
///   @endcode
///
/// Combines nicely with the waveform functions (like sin8(), etc)
/// to produce continuous hue gradients back and forth:
///   @code
///   hue = map8( sin8( myValue), HUE_BLUE, HUE_RED);
///   @endcode
///
/// Mathematically simiar to lerp8by8(), but arguments are more
/// like Arduino's "map"; this function is similar to
///   @code
///   map( in, 0, 255, rangeStart, rangeEnd)
///   @endcode
///
/// but faster and specifically designed for 8-bit values.
static uint8_t map8(uint8_t in, uint8_t rangeStart, uint8_t rangeEnd) {
  uint8_t rangeWidth = rangeEnd - rangeStart;
  uint8_t out = scale8(in, rangeWidth);
  out += rangeStart;
  return out;
}

/// Pre-calculated lookup table used in sin8() and cos8() functions
const uint8_t b_m16_interleave[] = {0, 49, 49, 41, 90, 27, 117, 10};

/// Fast 8-bit approximation of sin(x). This approximation never varies more
/// than 2% from the floating point value you'd get by doing
///   @code{.cpp}
///   float s = (sin(x) * 128.0) + 128;
///   @endcode
///
/// @param theta input angle from 0-255
/// @returns sin of theta, value between 0 and 255
static uint8_t sin8(uint8_t theta) {
  uint8_t offset = theta;
  if (theta & 0x40) {
    offset = (uint8_t)255 - offset;
  }
  offset &= 0x3F; // 0..63

  uint8_t secoffset = offset & 0x0F; // 0..15
  if (theta & 0x40)
    ++secoffset;

  uint8_t section = offset >> 4; // 0..3
  uint8_t s2 = section * 2;
  const uint8_t *p = b_m16_interleave;
  p += s2;
  uint8_t b = *p;
  ++p;
  uint8_t m16 = *p;

  uint8_t mx = (m16 * secoffset) >> 4;

  int8_t y = mx + b;
  if (theta & 0x80)
    y = -y;

  y += 128;

  return y;
}

/// Fast 8-bit approximation of cos(x). This approximation never varies more
/// than 2% from the floating point value you'd get by doing
///   @code{.cpp}
///   float s = (cos(x) * 128.0) + 128;
///   @endcode
///
/// @param theta input angle from 0-255
/// @returns cos of theta, value between 0 and 255
static uint8_t cos8(uint8_t theta) { return sin8(theta + 64); }

/// Fast 16-bit approximation of sin(x). This approximation never varies more
/// than 0.69% from the floating point value you'd get by doing
///    @code{.cpp}
///    float s = sin(x) * 32767.0;
///    @endcode
///
/// @param theta input angle from 0-65535
/// @returns sin of theta, value between -32767 to 32767.
static int16_t sin16(uint16_t theta) {
  static const uint16_t base[] = {0,     6393,  12539, 18204,
                                  23170, 27245, 30273, 32137};
  static const uint8_t slope[] = {49, 48, 44, 38, 31, 23, 14, 4};

  uint16_t offset = (theta & 0x3FFF) >> 3; // 0..2047
  if (theta & 0x4000)
    offset = 2047 - offset;

  uint8_t section = offset / 256; // 0..7
  uint16_t b = base[section];
  uint8_t m = slope[section];

  uint8_t secoffset8 = (uint8_t)(offset) / 2;

  uint16_t mx = m * secoffset8;
  int16_t y = mx + b;

  if (theta & 0x8000)
    y = -y;

  return y;
}

/// Fast 16-bit approximation of cos(x). This approximation never varies more
/// than 0.69% from the floating point value you'd get by doing
///    @code{.cpp}
///    float s = cos(x) * 32767.0;
///    @endcode
///
/// @param theta input angle from 0-65535
/// @returns cos of theta, value between -32767 to 32767.
static int16_t cos16(uint16_t theta) { return sin16(theta + 16384); }

///////////////////////////////////////////////////////////////////////
///
/// @defgroup BeatGenerators Waveform Beat Generators
/// Waveform generators that reset at a given number
/// of "beats per minute" (BPM).
///
/// The standard "beat" functions generate "sawtooth" waves which rise from
/// 0 up to a max value and then reset, continuously repeating that cycle at
/// the specified frequency (BPM).
///
/// The "sin" versions function similarly, but create an oscillating sine wave
/// at the specified frequency.
///
/// BPM can be supplied two ways. The simpler way of specifying BPM is as
/// a simple 8-bit integer from 1-255, (e.g., "120").
/// The more sophisticated way of specifying BPM allows for fractional
/// "Q8.8" fixed point number (an ::accum88) with an 8-bit integer part and
/// an 8-bit fractional part.  The easiest way to construct this is to multiply
/// a floating point BPM value (e.g. 120.3) by 256, (e.g. resulting in 30796
/// in this case), and pass that as the 16-bit BPM argument.
///
/// Originally these functions were designed to make an entire animation project
/// pulse. with brightness. For that effect, add this line just above your
/// existing call to "FastLED.show()":
///   @code
///   uint8_t bright = beatsin8( 60 /*BPM*/, 192 /*dimmest*/, 255 /*brightest*/
///   )); FastLED.setBrightness( bright ); FastLED.show();
///   @endcode
///
/// The entire animation will now pulse between brightness 192 and 255 once per
/// second.
///
/// @warning Any "BPM88" parameter **MUST** always be provided in Q8.8 format!
/// @note The beat generators need access to a millisecond counter
/// to track elapsed time. See ::GET_MILLIS for reference. When using the
/// Arduino `millis()` function, accuracy is a bit better than one part in a
/// thousand.
///
/// @{

/// Generates a 16-bit "sawtooth" wave at a given BPM, with BPM
/// specified in Q8.8 fixed-point format.
/// @param beats_per_minute_88 the frequency of the wave, in Q8.8 format
/// @param timebase the time offset of the wave from the millis() timer
/// @warning The BPM parameter **MUST** be provided in Q8.8 format! E.g.
/// for 120 BPM it would be 120*256 = 30720. If you just want to specify
/// "120", use beat16() or beat8().
static uint16_t beat88(accum88 beats_per_minute_88, uint32_t timebase = 0) {
  // BPM is 'beats per minute', or 'beats per 60000ms'.
  // To avoid using the (slower) division operator, we
  // want to convert 'beats per 60000ms' to 'beats per 65536ms',
  // and then use a simple, fast bit-shift to divide by 65536.
  //
  // The ratio 65536:60000 is 279.620266667:256; we'll call it 280:256.
  // The conversion is accurate to about 0.05%, more or less,
  // e.g. if you ask for "120 BPM", you'll get about "119.93".
  return ((GET_MILLIS() - timebase) * beats_per_minute_88 * 280) >> 16;
}

/// Generates a 16-bit "sawtooth" wave at a given BPM
/// @param beats_per_minute the frequency of the wave, in decimal
/// @param timebase the time offset of the wave from the millis() timer
static uint16_t beat16(accum88 beats_per_minute, uint32_t timebase = 0) {
  // Convert simple 8-bit BPM's to full Q8.8 accum88's if needed
  if (beats_per_minute < 256)
    beats_per_minute <<= 8;
  return beat88(beats_per_minute, timebase);
}

/// Generates an 8-bit "sawtooth" wave at a given BPM
/// @param beats_per_minute the frequency of the wave, in decimal
/// @param timebase the time offset of the wave from the millis() timer
static uint8_t beat8(accum88 beats_per_minute, uint32_t timebase = 0) {
  return beat16(beats_per_minute, timebase) >> 8;
}

/// Generates a 16-bit sine wave at a given BPM that oscillates within
/// a given range.
/// @param beats_per_minute_88 the frequency of the wave, in Q8.8 format
/// @param lowest the lowest output value of the sine wave
/// @param highest the highest output value of the sine wave
/// @param timebase the time offset of the wave from the millis() timer
/// @param phase_offset phase offset of the wave from the current position
/// @warning The BPM parameter **MUST** be provided in Q8.8 format! E.g.
/// for 120 BPM it would be 120*256 = 30720. If you just want to specify
/// "120", use beatsin16() or beatsin8().
static uint16_t beatsin88(accum88 beats_per_minute_88, uint16_t lowest = 0,
                          uint16_t highest = 65535, uint32_t timebase = 0,
                          uint16_t phase_offset = 0) {
  uint16_t beat = beat88(beats_per_minute_88, timebase);
  uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
  uint16_t rangewidth = highest - lowest;
  uint16_t scaledbeat = scale16(beatsin, rangewidth);
  uint16_t result = lowest + scaledbeat;
  return result;
}

/// Generates a 16-bit sine wave at a given BPM that oscillates within
/// a given range.
/// @param beats_per_minute the frequency of the wave, in decimal
/// @param lowest the lowest output value of the sine wave
/// @param highest the highest output value of the sine wave
/// @param timebase the time offset of the wave from the millis() timer
/// @param phase_offset phase offset of the wave from the current position
static uint16_t beatsin16(accum88 beats_per_minute, uint16_t lowest = 0,
                          uint16_t highest = 65535, uint32_t timebase = 0,
                          uint16_t phase_offset = 0) {
  uint16_t beat = beat16(beats_per_minute, timebase);
  uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
  uint16_t rangewidth = highest - lowest;
  uint16_t scaledbeat = scale16(beatsin, rangewidth);
  uint16_t result = lowest + scaledbeat;
  return result;
}

/// Generates an 8-bit sine wave at a given BPM that oscillates within
/// a given range.
/// @param beats_per_minute the frequency of the wave, in decimal
/// @param lowest the lowest output value of the sine wave
/// @param highest the highest output value of the sine wave
/// @param timebase the time offset of the wave from the millis() timer
/// @param phase_offset phase offset of the wave from the current position
static uint8_t beatsin8(accum88 beats_per_minute, uint8_t lowest = 0,
                        uint8_t highest = 255, uint32_t timebase = 0,
                        uint8_t phase_offset = 0) {
  uint8_t beat = beat8(beats_per_minute, timebase);
  uint8_t beatsin = sin8(beat + phase_offset);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}
} // namespace ravecylinder

#endif