#ifndef WLED_FCN_DECLARE_H
#define WLED_FCN_DECLARE_H

#include <stdint.h>
#include <math.h>

// WLED_json.cpp
#include <httpserver.hpp>
//#include "src/dependencies/json/ArduinoJson-v6.h"
//#include "src/dependencies/json/AsyncJson-v6.h"
#include "WLED_FX.h"

namespace ravecylinder {
inline uint8_t pgm_read_byte_near(uint8_t *p) { return *p; }
inline uint8_t pgm_read_byte_near(const unsigned char *p) { return *p; }
// similar to NeoPixelBus NeoGammaTableMethod but allows dynamic changes
// (superseded by NPB::NeoGammaDynamicTableMethod)
class NeoGammaWLEDMethod {
public:
  static uint8_t Correct(uint8_t value);     // apply Gamma to single channel
  static uint32_t Correct32(uint32_t color); // apply Gamma to RGBW32 color
                                             // (WLED specific, not used by NPB)
  static void calcGammaTable(float gamma); // re-calculates & fills gamma table
  static inline uint8_t rawGamma8(uint8_t val) {
    return gammaT[val];
  } // get value from Gamma table (WLED specific, not used by NPB)
private:
  static uint8_t gammaT[];
};
#define gamma32(c) NeoGammaWLEDMethod::Correct32(c)
#define gamma8(c) NeoGammaWLEDMethod::rawGamma8(c)

uint32_t color_blend(uint32_t, uint32_t, uint16_t, bool b16 = false);
uint32_t color_add(uint32_t, uint32_t, bool fast = false);
uint32_t color_fade(uint32_t c1, uint8_t amount, bool video = false);
inline uint32_t colorFromRgbw(uint8_t *rgbw) {
  return uint32_t((uint8_t(rgbw[3]) << 24) | (uint8_t(rgbw[0]) << 16) |
                  (uint8_t(rgbw[1]) << 8) | (uint8_t(rgbw[2])));
}
void colorHStoRGB(uint16_t hue, uint8_t sat, uint8_t *rgb); // hue, sat to rgb
void colorKtoRGB(uint16_t kelvin, uint8_t *rgb);
void colorCTtoRGB(uint16_t mired, uint8_t *rgb); // white spectrum to rgb
void colorXYtoRGB(float x, float y,
                  uint8_t *rgb); // only defined if huesync disabled TODO
void colorRGBtoXY(uint8_t *rgb,
                  float *xy); // only defined if huesync disabled TODO
void colorFromDecOrHexString(uint8_t *rgb, char *in);
bool colorFromHexString(uint8_t *rgb, const char *in);
uint32_t colorBalanceFromKelvin(uint16_t kelvin, uint32_t rgb);
uint16_t approximateKelvinFromRGB(uint32_t rgb);
void setRandomColor(uint8_t *rgb);
//uint8_t gamma8(uint8_t b);
int16_t extractModeDefaults(uint8_t mode, const char *segVar);

// Perlin noise functions
uint8_t inoise8(uint16_t x, uint16_t y, uint16_t z);
uint8_t inoise8(uint16_t x, uint16_t y);
uint8_t inoise8(uint16_t x);

int8_t inoise8_raw(uint16_t x, uint16_t y, uint16_t z);
int8_t inoise8_raw(uint16_t x, uint16_t y);
int8_t inoise8_raw(uint16_t x);

uint16_t inoise16(uint32_t x, uint32_t y, uint32_t z);
uint16_t inoise16(uint32_t x, uint32_t y);
uint16_t inoise16(uint32_t x);

int16_t inoise16_raw(uint32_t x, uint32_t y, uint32_t z);
int16_t inoise16_raw(uint32_t x, uint32_t y);
int16_t inoise16_raw(uint32_t x);

#define sin_t sin
#define cos_t cos
#define tan_t tan
#define asin_t asin
#define acos_t acos
#define atan_t atan
#define fmod_t fmod
#define floor_t floor

//um_manager.cpp
typedef enum UM_Data_Types {
  UMT_BYTE = 0,
  UMT_UINT16,
  UMT_INT16,
  UMT_UINT32,
  UMT_INT32,
  UMT_FLOAT,
  UMT_DOUBLE,
  UMT_BYTE_ARR,
  UMT_UINT16_ARR,
  UMT_INT16_ARR,
  UMT_UINT32_ARR,
  UMT_INT32_ARR,
  UMT_FLOAT_ARR,
  UMT_DOUBLE_ARR
} um_types_t;
typedef struct UM_Exchange_Data {
  // should just use: size_t arr_size, void **arr_ptr, byte *ptr_type
  size_t       u_size;                 // size of u_data array
  um_types_t  *u_type;                 // array of data types
  void       **u_data;                 // array of pointers to data
  UM_Exchange_Data() {
    u_size = 0;
    u_type = nullptr;
    u_data = nullptr;
  }
  ~UM_Exchange_Data() {
    if (u_type) delete[] u_type;
    if (u_data) delete[] u_data;
  }
} um_data_t;
const unsigned int um_data_size = sizeof(um_data_t);  // 12 bytes

class Usermods {
  protected:
  public:
    Usermods() { }
    virtual ~Usermods() { }
    virtual bool getUMData(um_data_t **data, int id) { return false; };
};
extern Usermods usermods;
um_data_t* simulateSound(uint8_t simulationId);
CHSV rgb2hsv_approximate( const CRGB& rgb);

//json.cpp
// bool deserializeSegment(json& elem, byte it, byte presetId = 0);
// bool deserializeState(JsonObject root, byte callMode = CALL_MODE_DIRECT_CHANGE, byte presetId = 0);
// void serializeSegment(JsonObject& root, Segment& seg, byte id, bool forPreset = false, bool segmentBounds = true);
// void serializeState(JsonObject root, bool forPreset = false, bool includeBri = true, bool segmentBounds = true, bool selectedSegmentsOnly = false);
// void serializeInfo(JsonObject root);
// void serializeModeNames(JsonArray root);
// void serializeModeData(JsonArray root);
// #ifdef WLED_ENABLE_JSONLIVE
// bool serveLiveLeds(AsyncWebServerRequest* request, uint32_t wsClient = 0);
// #endif



} // namespace ravecylinder
#endif