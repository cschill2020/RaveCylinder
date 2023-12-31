#ifndef WLED_H
#define WLED_H

#include "Pixel.h"
#include "WLED_const.h"
#include <cstdint>
// #include "fcn_declare.h"
// #include "NodeStruct.h"
// #include "pin_manager.h"
// #include "bus_manager.h"
// #include "WLED_FX.h"

namespace ravecylinder {

#define _INIT(x) = x

#define NUM_PIXELS 20
extern CRGB _pixels[];

// extern WS2812FX strip _INIT(WS2812FX());

typedef uint8_t byte;

#define USER_PRINT(a)
#define F(x...)
#define DEBUG_PRINT(x...)
#define DEBUG_PRINTLN(a)
#define DEBUG_PRINTF(x...)

#define PROGMEM
#define IRAM_ATTR

#define PSTR(a) a
#define strncmp_P strncmp
#define sprintf_P sprintf
#define memcpy_P memcpy
#define snprintf_P snprintf
#define strcpy_P strcpy
#define strcat_P strcpy

#define pgm_read_byte(x) (*((const uint8_t *)(x)))
#define pgm_read_word(x) (*((const uint16_t *)(x)))
#define pgm_read_dword(x) (*((const uint32_t *)(x)))

constexpr float HALF_PI = 3.14159f / 2.0f;

extern bool stateChanged;
extern uint8_t randomPaletteChangeTime;

constexpr bool modeBlending = true;
constexpr bool fadeTransition = false;
constexpr bool gammaCorrectCol = false;
constexpr bool useAMPM = false;
constexpr bool correctWB = false;
constexpr bool autoSegments = false;
constexpr bool gammaCorrectBri = false;
constexpr bool cctFromRgb = false;
extern uint16_t ledMaps;

// color mangling macros
#define RGBW32(r, g, b, w)                                                     \
  (uint32_t((uint8_t(w) << 24) | (uint8_t(r) << 16) | (uint8_t(g) << 8) |      \
            (uint8_t(b))))
#define R(c) (uint8_t((c) >> 16))
#define G(c) (uint8_t((c) >> 8))
#define B(c) (uint8_t(c))
#define W(c) (uint8_t((c) >> 24))

#define yield() 

// color
extern byte lastRandomIndex;
extern char* ledmapNames[];

#define wled_map(a1, a2, b1, b2, s) (b1 + (s - a1) * (b2 - b1) / (a2 - a1))

void colorHStoRGB(uint16_t hue, byte sat, byte* rgb);
uint32_t gamma32(uint32_t);

class BusConfig {
public:
  BusConfig(...) {}
};
class Bus {
public:
  int getStart() { return 0; }
  int getType() { return 0; }
  bool hasWhite() { return false; }
  bool hasRGB() { return true; }

  int getLength();
  uint32_t getPixelColor(int i);
  void setPixelColor(int i, uint32_t c);

  bool hasCCT() { return false; }
  bool isOk() { return true; }
  uint8_t getAutoWhiteMode() { return 0; }
  static uint8_t getGlobalAWMode() { return 0; }
  bool isOffRefreshRequired() { return false; }
};
class BusManager {
public:
  uint32_t getNumBusses() const { return 1; }
  Bus *getBus(uint32_t b) { return &bus; }
  void setBrightness(uint32_t b) {}
  bool hasRGB() { return true; }
  bool hasWhite() { return false; }
  bool canAllShow() { return false; }
  void show() {}
  void setSegmentCCT(int i, bool b = false) {}
  uint32_t getPixelColor(int i) { return bus.getPixelColor(i); }
  void setPixelColor(int i, uint32_t c) { bus.setPixelColor(i, c); }
  int add(...) { return -1; }

  Bus bus;
};

class WLEDFileSystem {
public:
  bool exists(...) { return false; }
};
extern WLEDFileSystem WLED_FS;

} // namespace ravecylinder

#endif