#include "WLED.h"
#include "WLED_FX.h"
#include "WLED_fcn_declare.h"

namespace ravecylinder {

// uint16_t rand16seed = 0;
time_t localTime = time(nullptr);
bool stateChanged = false;
uint16_t ledMaps = 0xFFFF;
CRGB _pixels[NUM_PIXELS];

// Global variable extern initialization

// LED CONFIG
byte nightlightTargetBri = 0; // brightness after nightlight is over
byte nightlightDelayMins = 60;
byte nightlightMode =
    NL_MODE_FADE; // See const.h for available modes. Was nightlightFade

// SYNC CONFIG
bool notifyDirect = false; // send notification if change via UI or HTTP API

// color
byte lastRandomIndex = 0;

// transitions
bool fadeTransition = true;
bool modeBlending = true;
bool transitionActive = false;
uint16_t transitionDelay = 750;
uint16_t transitionDelayDefault = 750;
unsigned long transitionStartTime;
float tperLast = 0.0f;
bool jsonTransitionOnce = false;
uint8_t randomPaletteChangeTime = 5;

// nightlight
bool nightlightActive = false;
bool nightlightActiveOld = false;
uint32_t nightlightDelayMs = 10;
byte nightlightDelayMinsDefault = nightlightDelayMins;
unsigned long nightlightStartTime;
byte briNlT = 0;                // current nightlight brightness
byte *colNlT[4] = {0, 0, 0, 0}; // current nightlight color

// brightness
byte bri = 128;
byte briLast = 128;

// playlists
int16_t currentPlaylist = -1;

// presets
byte currentPreset = 0;
byte errorFlag = 0;

// led fx library object
char *ledmapNames[WLED_MAX_LEDMAPS] = {nullptr};
BusManager busses;
WLEDFileSystem WLED_FS;

int16_t extractModeDefaults(uint8_t mode, const char *segVar) {
  if (mode < strip().getModeCount()) {
    char lineBuffer[128] = "";
    strncpy(lineBuffer, strip().getModeData(mode), 127);
    lineBuffer[127] = '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char *startPtr = strrchr(lineBuffer, ';'); // last ";" in FX data
      if (!startPtr)
        return -1;

      char *stopPtr = strstr(startPtr, segVar);
      if (!stopPtr)
        return -1;

      stopPtr += strlen(segVar) + 1; // skip "="
      return atoi(stopPtr);
    }
  }
  return -1;
}

void WS2812FX::loadCustomPalettes() {}

int Bus::getLength() { return NUM_PIXELS; }

uint32_t Bus::getPixelColor(int i) {
  return (_pixels[i].r << 16) | (_pixels[i].g << 8) | _pixels[i].b;
}

void Bus::setPixelColor(int i, uint32_t c) {
  int r = R(c);
  int g = G(c);
  int b = B(c);
  _pixels[i].r = r;
  _pixels[i].g = g;
  _pixels[i].b = b;
}
} // namespace ravecylinder