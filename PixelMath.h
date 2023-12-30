// Taken from FastLED.

#ifndef __PIXELMATH_H
#define __PIXELMATH_H

#include <chrono>
#include <iostream>
#include <stdint.h>

namespace ravecylinder {
using clock_type = std::chrono::high_resolution_clock;
typedef uint16_t accum88;
typedef uint8_t fract8;
typedef uint16_t fract16;

uint32_t get_millisecond_timer();

#define GET_MILLIS get_millisecond_timer
#define millis get_millisecond_timer

static float constrain(float x, float low, float high) {
  if (x < low) {
    return low;
  } else if (x > high) {
    return high;
  } else {
    return x;
  }
}

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

static uint8_t sqrt16(uint16_t x) {
  if (x <= 1) {
    return x;
  }

  uint8_t low = 1; // lower bound
  uint8_t hi, mid;

  if (x > 7904) {
    hi = 255;
  } else {
    hi = (x >> 5) + 8; // initial estimate for upper bound
  }

  do {
    mid = (low + hi) >> 1;
    if ((uint16_t)(mid * mid) > x) {
      hi = mid - 1;
    } else {
      if (mid == 255) {
        return 255;
      }
      low = mid + 1;
    }
  } while (hi >= low);

  return low - 1;
}

// Scale is unit of scale/256.
static uint8_t scale8(uint8_t i, uint8_t scale) {
  return (((uint16_t)i) * (1 + (uint16_t)(scale))) >> 8;
}

static uint8_t scale8_video(uint8_t i, uint8_t scale) {
  return (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
}

static uint16_t scale16(uint16_t i, fract16 scale) {
  return ((uint32_t)(i) * (1 + (uint32_t)(scale))) / 65536;
}

/// Add one byte to another, saturating at 0xFF
/// @param i first byte to add
/// @param j second byte to add
/// @returns the sum of i + j, capped at 0xFF
static inline uint8_t qadd8(uint8_t i, uint8_t j) {
  unsigned int t = i + j;
  if (t > 255)
    t = 255;
  return t;
}

/// Add one byte to another, with 8-bit result
/// @note This does not saturate and may overflow!
/// @param i first byte to add
/// @param j second byte to add
/// @returns the sum of i + j, 8-bit
static inline uint8_t add8(uint8_t i, uint8_t j) {
  int t = i + j;
  return t;
}

/// Subtract one byte from another, saturating at 0x00
/// @param i byte to subtract from
/// @param j byte to subtract
/// @returns i - j with a floor of 0
static inline uint8_t qsub8(uint8_t i, uint8_t j) {
  int t = i - j;
  if (t < 0)
    t = 0;
  return t;
}

/// Calculate an integer average of two signed 7-bit
/// integers (int8_t).
/// If the first argument is even, result is rounded down.
/// If the first argument is odd, result is rounded up.
/// @param i first value to average
/// @param j second value to average
/// @returns mean average of i and j, rounded
static inline int8_t avg7(int8_t i, int8_t j) {
  return (i >> 1) + (j >> 1) + (i & 0x1);
}

/// Calculate an integer average of two signed 15-bit
/// integers (int16_t).
/// If the first argument is even, result is rounded down.
/// If the first argument is odd, result is rounded up.
/// @param i first value to average
/// @param j second value to average
/// @returns mean average of i and j, rounded
static inline int16_t avg15(int16_t i, int16_t j) {
  return (i >> 1) + (j >> 1) + (i & 0x1);
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

// Random number generation;
/// Multiplier value for pseudo-random number generation
#define FASTLED_RAND16_2053 ((uint16_t)(2053))
/// Increment value for pseudo-random number generation
#define FASTLED_RAND16_13849 ((uint16_t)(13849))
/// Multiplies a value by the pseudo-random multiplier
#define APPLY_FASTLED_RAND16_2053(x) (x * FASTLED_RAND16_2053)
/// Seed for the random number generator functions
static uint16_t rand16seed = 1337;

static uint16_t random16() {
  rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
  return rand16seed;
}

static uint8_t random8() {
  rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
  // return the sum of the high and low bytes, for better
  //  mixing and non-sequential correlation
  return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                   ((uint8_t)(rand16seed >> 8)));
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result, exclusive
static uint8_t random8(uint8_t lim) {
  uint8_t r = random8();
  r = (r * lim) >> 8;
  return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number, inclusive
/// @param lim the upper bound for the random number, exclusive
static uint8_t random8(uint8_t min, uint8_t lim) {
  uint8_t delta = lim - min;
  uint8_t r = random8(delta) + min;
  return r;
}

/// Generate an 16-bit random number between 0 and lim
/// @param lim the upper bound for the result, exclusive
static uint16_t random16(uint16_t lim) {
  uint16_t r = random16();
  uint32_t p = (uint32_t)lim * (uint32_t)r;
  r = p >> 16;
  return r;
}

/// Generate an 16-bit random number in the given range
/// @param min the lower bound for the random number, inclusive
/// @param lim the upper bound for the random number, exclusive
static uint16_t random16(uint16_t min, uint16_t lim) {
  uint16_t delta = lim - min;
  uint16_t r = random16(delta) + min;
  return r;
}

/// Set the 16-bit seed used for the random number generator
static void random16_set_seed(uint16_t seed) { rand16seed = seed; }

/// Get the current seed value for the random number generator
static uint16_t random16_get_seed() { return rand16seed; }

/// Add entropy into the random number generator
static void random16_add_entropy(uint16_t entropy) { rand16seed += entropy; }

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

/// Linear interpolation between two unsigned 8-bit values,
/// with 8-bit fraction
static uint8_t lerp8by8(uint8_t a, uint8_t b, fract8 frac) {
  uint8_t result;
  if (b > a) {
    uint8_t delta = b - a;
    uint8_t scaled = scale8(delta, frac);
    result = a + scaled;
  } else {
    uint8_t delta = a - b;
    uint8_t scaled = scale8(delta, frac);
    result = a - scaled;
  }
  return result;
}
/// Linear interpolation between two signed 15-bit values,
/// with 8-bit fraction
static int16_t lerp15by16(int16_t a, int16_t b, fract16 frac) {
  int16_t result;
  if (b > a) {
    uint16_t delta = b - a;
    uint16_t scaled = scale16(delta, frac);
    result = a + scaled;
  } else {
    uint16_t delta = a - b;
    uint16_t scaled = scale16(delta, frac);
    result = a - scaled;
  }
  return result;
}

/// 8-bit quadratic ease-in / ease-out function.
/// Takes around 13 cycles on AVR.
static uint8_t ease8InOutQuad(uint8_t i) {
  uint8_t j = i;
  if (j & 0x80) {
    j = 255 - j;
  }
  uint8_t jj = scale8(j, j);
  uint8_t jj2 = jj << 1;
  if (i & 0x80) {
    jj2 = 255 - jj2;
  }
  return jj2;
}

static uint16_t ease16InOutQuad(uint16_t i) {
  uint16_t j = i;
  if (j & 0x8000) {
    j = 65535 - j;
  }
  uint16_t jj = scale16(j, j);
  uint16_t jj2 = jj << 1;
  if (i & 0x8000) {
    jj2 = 65535 - jj2;
  }
  return jj2;
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

static uint16_t crc16(const unsigned char *data_p, size_t length) {
  uint8_t x;
  uint16_t crc = 0xFFFF;
  if (!length)
    return 0x1D0F;
  while (length--) {
    x = crc >> 8 ^ *data_p++;
    x ^= x >> 4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x << 5)) ^
          ((uint16_t)x);
  }
  return crc;
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

/// Return the current seconds since boot in a 16-bit value.  Used as part of
/// the "every N time-periods" mechanism
static uint16_t seconds16() {
  uint32_t ms = GET_MILLIS();
  uint16_t s16;
  s16 = ms / 1000;
  return s16;
}

/// Return the current minutes since boot in a 16-bit value.  Used as part of
/// the "every N time-periods" mechanism
static uint16_t minutes16() {
  uint32_t ms = GET_MILLIS();
  uint16_t m16;
  m16 = (ms / (60000L)) & 0xFFFF;
  return m16;
}

/// Return the current hours since boot in an 8-bit value.  Used as part of the
/// "every N time-periods" mechanism
static uint8_t hours8() {
  uint32_t ms = GET_MILLIS();
  uint8_t h8;
  h8 = (ms / (3600000L)) & 0xFF;
  return h8;
}

/// 8-bit cubic ease-in / ease-out function.
/// Takes around 18 cycles on AVR.
static fract8 ease8InOutCubic(fract8 i) {
  uint8_t ii = scale8(i, i);
  uint8_t iii = scale8(ii, i);

  uint16_t r1 = (3 * (uint16_t)(ii)) - (2 * (uint16_t)(iii));

  /* the code generated for the above *'s automatically
     cleans up R1, so there's no need to explicitily call
     cleanup_R1(); */

  uint8_t result = r1;

  // if we got "256", return 255:
  if (r1 & 0x100) {
    result = 255;
  }
  return result;
}

/// Fast, rough 8-bit ease-in/ease-out function.
/// Shaped approximately like ease8InOutCubic(),
/// it's never off by more than a couple of percent
/// from the actual cubic S-curve, and it executes
/// more than twice as fast.  Use when the cycles
/// are more important than visual smoothness.
/// Asm version takes around 7 cycles on AVR.
static fract8 ease8InOutApprox(fract8 i) {
  if (i < 64) {
    // start with slope 0.5
    i /= 2;
  } else if (i > (255 - 64)) {
    // end with slope 0.5
    i = 255 - i;
    i /= 2;
    i = 255 - i;
  } else {
    // in the middle, use slope 192/128 = 1.5
    i -= 64;
    i += (i / 2);
    i += 32;
  }

  return i;
}

///////////////////////////////////////////////////////////////////////
///
/// @defgroup WaveformGenerators Waveform Generators
/// General purpose wave generator functions.
///
/// @{

/// Triangle wave generator.
/// Useful for turning a one-byte ever-increasing value into a
/// one-byte value that oscillates up and down.
///   @code
///           input         output
///           0..127        0..254 (positive slope)
///           128..255      254..0 (negative slope)
///   @endcode
///
/// On AVR this function takes just three cycles.
///
static uint8_t triwave8(uint8_t in) {
  if (in & 0x80) {
    in = 255 - in;
  }
  uint8_t out = in << 1;
  return out;
}

/// Quadratic waveform generator. Spends just a little
/// more time at the limits than "sine" does.
///
/// S-shaped wave generator (like "sine"). Useful
/// for turning a one-byte "counter" value into a
/// one-byte oscillating value that moves smoothly up and down,
/// with an "acceleration" and "deceleration" curve.
///
/// This is even faster than "sin8()", and has
/// a slightly different curve shape.
static uint8_t quadwave8(uint8_t in) { return ease8InOutQuad(triwave8(in)); }

/// Cubic waveform generator. Spends visibly more time
/// at the limits than "sine" does.
/// @copydetails quadwave8()
static uint8_t cubicwave8(uint8_t in) { return ease8InOutCubic(triwave8(in)); }

/// Square wave generator.
/// Useful for turning a one-byte ever-increasing value
/// into a one-byte value that is either 0 or 255.
/// The width of the output "pulse" is determined by
/// the pulsewidth argument:
///   @code
///   if pulsewidth is 255, output is always 255.
///   if pulsewidth < 255, then
///     if input < pulsewidth  then output is 255
///     if input >= pulsewidth then output is 0
///   @endcode
///
/// The output looking like:
///
///   @code
///     255   +--pulsewidth--+
///      .    |              |
///      0    0              +--------(256-pulsewidth)--------
///   @endcode
///
/// @param in input value
/// @param pulsewidth width of the output pulse
/// @returns square wave output
static uint8_t squarewave8(uint8_t in, uint8_t pulsewidth = 128) {
  if (in < pulsewidth || (pulsewidth == 255)) {
    return 255;
  } else {
    return 0;
  }
}

/// @} WaveformGenerators
// Under C++11 rules, we would be allowed to use not-external
// -linkage-type symbols as template arguments,
// e.g., LIB8STATIC seconds16, and we'd be able to use these
// templates as shown below.
// However, under C++03 rules, we cannot do that, and thus we
// have to resort to the preprocessor to 'instantiate' 'templates',
// as handled above.
template <typename timeType, timeType (*timeGetter)()>
class CEveryNTimePeriods {
public:
  timeType mPrevTrigger;
  timeType mPeriod;

  CEveryNTimePeriods() {
    reset();
    mPeriod = 1;
  };
  CEveryNTimePeriods(timeType period) {
    reset();
    setPeriod(period);
  };
  void setPeriod(timeType period) { mPeriod = period; };
  timeType getTime() { return (timeType)(timeGetter()); };
  timeType getPeriod() { return mPeriod; };
  timeType getElapsed() { return getTime() - mPrevTrigger; }
  timeType getRemaining() { return mPeriod - getElapsed(); }
  timeType getLastTriggerTime() { return mPrevTrigger; }
  bool ready() {
    bool isReady = (getElapsed() >= mPeriod);
    if (isReady) {
      reset();
    }
    return isReady;
  }
  void reset() { mPrevTrigger = getTime(); };
  void trigger() { mPrevTrigger = getTime() - mPeriod; };

  operator bool() { return ready(); }
};
typedef CEveryNTimePeriods<uint16_t, seconds16> CEveryNSeconds;
typedef CEveryNTimePeriods<uint32_t, get_millisecond_timer> CEveryNMillis;
typedef CEveryNTimePeriods<uint16_t, minutes16> CEveryNMinutes;
typedef CEveryNTimePeriods<uint8_t, hours8> CEveryNHours;

/// @name "EVERY_N_TIME" Macros
/// Check whether to excecute a block of code every N amount of time.
/// These are useful for limiting how often code runs. For example,
/// you can use ::fill_rainbow() to fill a strip of LEDs with color,
/// combined with an ::EVERY_N_MILLIS block to limit how fast the colors
/// change:
///   @code{.cpp}
///   static uint8_t hue = 0;
///   fill_rainbow(leds, NUM_LEDS, hue);
///   EVERY_N_MILLIS(20) { hue++; }  // advances hue every 20 milliseconds
///   @endcode
/// Note that in order for these to be accurate, the EVERY_N block must
/// be evaluated at a regular basis.
/// @{

/// @cond
#define CONCAT_HELPER(x, y) x##y
#define CONCAT_MACRO(x, y) CONCAT_HELPER(x, y)
/// @endcond

/// Checks whether to execute a block of code every N milliseconds
/// @see GET_MILLIS
#define EVERY_N_MILLIS(N) EVERY_N_MILLIS_I(CONCAT_MACRO(PER, __COUNTER__), N)

/// Checks whether to execute a block of code every N milliseconds, using a
/// custom instance name
/// @copydetails EVERY_N_MILLIS
#define EVERY_N_MILLIS_I(NAME, N)                                              \
  static CEveryNMillis NAME(N);                                                \
  if (NAME)

/// Checks whether to execute a block of code every N seconds
/// @see seconds16()
#define EVERY_N_SECONDS(N) EVERY_N_SECONDS_I(CONCAT_MACRO(PER, __COUNTER__), N)

/// Checks whether to execute a block of code every N seconds, using a custom
/// instance name
/// @copydetails EVERY_N_SECONDS
#define EVERY_N_SECONDS_I(NAME, N)                                             \
  static CEveryNSeconds NAME(N);                                               \
  if (NAME)

} // namespace ravecylinder

#endif