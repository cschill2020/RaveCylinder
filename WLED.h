#ifndef WLED_H
#define WLED_H

#include "Pixel.h"
#include "DDPOutput.h"
#include "UDPClient.h"
#include "WLED_const.h"

#include <cstdint>
#include <httpserver.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ravecylinder {
// Core globals
// Decided on static global function to init strip in WLED.cpp
// extern WS2812FX strip _INIT(WS2812FX());

// typedef and defines to override AVR specific instructions.
typedef uint8_t byte;

// Override anrduino fns to c++ or nothing.
#define USER_PRINT(a)
#define F(a) a
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
#define WLED_GLOBAL extern
#define pgm_read_byte(x) (*((const uint8_t *)(x)))
#define pgm_read_word(x) (*((const uint16_t *)(x)))
#define pgm_read_dword(x) (*((const uint32_t *)(x)))
#define SET_F

// Constants
constexpr float HALF_PI = 3.14159f / 2.0f;
constexpr bool useAMPM = false;
constexpr bool gammaCorrectBri = false;
constexpr bool cctFromRgb = false;
extern uint16_t ledMaps;

extern bool stateChanged;

#define WLED_yield()

long long GetTimeMicros(void);

inline uint64_t micros() { return GetTimeMicros(); }

// LED CONFIG
extern bool autoSegments;
extern byte nightlightTargetBri; // brightness after nightlight is over
extern byte nightlightDelayMins;
extern byte nightlightMode;     // See const.h for available modes. Was nightlightFade
extern byte bootPreset; // save preset to load after power-up
extern bool turnOnAtBoot;
extern unsigned long lastEditTime;
extern bool doSerializeConfig;
extern bool doReboot; // flag to initiate reboot from async handlers
extern bool correctWB;
extern bool doInitBusses;

// SYNC CONFIG
// Likely unused but here to limit javascript changes
WLED_GLOBAL bool notifyDirect; // send notification if change via UI or HTTP API

// color
extern byte lastRandomIndex;
extern bool gammaCorrectCol;
extern float gammaCorrectVal; // gamma correction value

// transitions
extern bool fadeTransition; // enable crossfading brightness/color
extern bool modeBlending;   // enable effect blending
extern bool transitionActive;
extern uint16_t transitionDelay; // global transition duration
extern uint16_t
    transitionDelayDefault; // default transition time (stored in cfg.json)
extern unsigned long transitionStartTime;
extern float tperLast;          // crossfade transition progress, 0.0f - 1.0f
extern bool jsonTransitionOnce; // flag to override transitionDelay (playlist,
                                // JSON API: "live" & "seg":{"i"} & "tt")
extern uint8_t randomPaletteChangeTime; // amount of time [s] between random
                                        // palette changes (min: 1s, max: 255s)

// nightlight
WLED_GLOBAL bool nightlightActive;
WLED_GLOBAL bool nightlightActiveOld;
WLED_GLOBAL uint32_t nightlightDelayMs;
WLED_GLOBAL byte nightlightDelayMinsDefault;
WLED_GLOBAL unsigned long nightlightStartTime;
WLED_GLOBAL byte briNlT;    // current nightlight brightness
WLED_GLOBAL byte *colNlT[]; // current nightlight color

// brightness
extern byte briS; // default brightness
extern byte bri;  // global brightness (set)
extern byte briLast;
extern byte
    briMultiplier; // % of brightness to set (to limit power, if you set it to
                   // 50 and set bri to 255, actual brightness will be 127)

// playlists
extern int16_t currentPlaylist;

// network
extern UDPClient udpClient;  // For sending DDP packets to the controller.
extern DDPOutput ddpOutput;
// presets
extern byte currentPreset;
extern byte errorFlag;

// led fx library object
extern char *ledmapNames[];

// Temp buffer
WLED_GLOBAL char *obuf;
WLED_GLOBAL uint16_t olen;

// GLOBAL FUNCTION DECLARATIONS

#define wled_map(a1, a2, b1, b2, s) (b1 + (s - a1) * (b2 - b1) / (a2 - a1))
// color mangling macros
#define RGBW32(r, g, b, w)                                                     \
  (uint32_t((uint8_t(w) << 24) | (uint8_t(r) << 16) | (uint8_t(g) << 8) |      \
            (uint8_t(b))))
#define R(c) (uint8_t((c) >> 16))
#define G(c) (uint8_t((c) >> 8))
#define B(c) (uint8_t(c))
#define W(c) (uint8_t((c) >> 24))

void colorHStoRGB(uint16_t hue, byte sat, byte *rgb);
uint32_t gamma32(uint32_t);

void getSettingsJS(int subPage, char *dest);
void handleSettingsSet(const httpserver::http_request &req, byte subPage);

// Bus Manager class for storing pixel information.  Not really doing
// bus stuff for Rpi, added for compatibility with WLED implementation.
class BusConfig {
public:
  BusConfig(uint16_t length = 200) : _len(length) {}
  uint16_t _len;
};

class Bus {
public:
  Bus(const uint16_t len = 200);
  int getStart() { return 0; }
  int getType() { return 22; }
  bool hasWhite() { return false; }
  bool hasRGB() { return true; }

  int getLength();
  uint32_t getPixelColor(int i);
  void setPixelColor(int i, uint32_t c);

  uint8_t getPins() { return 1; }

  bool hasCCT() { return false; }
  bool isOk() { return true; }
  uint8_t getAutoWhiteMode() { return 0; }
  static uint8_t getGlobalAWMode() { return 0; }
  bool isOffRefreshRequired() { return false; }

  CRGB* getPixels() { return _pixels; }

  private:
    CRGB* _pixels;
    uint16_t _len;

    CRGB *allocData(size_t size = 1);
};
class BusManager {
public:
  uint32_t getNumBusses() const { return 1; }
  Bus *getBus(uint32_t b) { return &bus; }
  void setBrightness(uint32_t b) {}
  bool hasRGB() { return true; }
  bool hasWhite() { return false; }
  bool canAllShow() { return false; }
  void show() {
    std::vector<Packet> packets =
        ddpOutput.GenerateFrame(bus.getPixels(), bus.getLength());
    for (auto &packet : packets) {
      // Send the packets.
      udpClient.SendTo(packet.GetBytes());
    }
  }
  void setSegmentCCT(int i, bool b = false) {}
  uint32_t getPixelColor(int i) { return bus.getPixelColor(i); }
  void setPixelColor(int i, uint32_t c) { bus.setPixelColor(i, c); }
  int add(...) { return -1; }

  Bus bus;
};

extern BusConfig busConfig;
extern BusManager busses;

// WLED FS is not used.  We are using the standard linux file-system.
// Added for compatibility with WLED implementations.
class WLEDFileSystem {
public:
  bool exists(...) { return false; }
};
extern WLEDFileSystem WLED_FS;

// Can these live in WLED_FX_fcn_declare.h?
// WLED_json.cpp
std::shared_ptr<httpserver::http_response>
serveJson(const httpserver::http_request &request);
std::shared_ptr<httpserver::http_response>
postJson(const httpserver::http_request &request);

// WLED_server.cpp
class index_response : public httpserver::http_resource {
public:
  std::shared_ptr<httpserver::http_response>
  render_GET(const httpserver::http_request &req);
};

class json_response : public httpserver::http_resource {
public:
  std::shared_ptr<httpserver::http_response>
  render_GET(const httpserver::http_request &req);
  std::shared_ptr<httpserver::http_response>
  render_POST(const httpserver::http_request &req);
};

class settings_response : public httpserver::http_resource {
public:
  std::shared_ptr<httpserver::http_response>
  render_GET(const httpserver::http_request &req);
  std::shared_ptr<httpserver::http_response>
  render_POST(const httpserver::http_request &req);
};

class Rave {
public:
  Rave() {}
  static Rave &instance() {
    static Rave instance;
    return instance;
  }

  // boot starts here
  void setup();

  void loop();
  void reset();

  void beginStrip();
  // void handleConnection();
  // bool initEthernet(); // result is informational
  // void initAP(bool resetAP = false);
  // void initConnection();
  // void initInterfaces();
  // void handleStatusLED();
  // void enableWatchdog();
  // void disableWatchdog();
};

} // namespace ravecylinder

#endif