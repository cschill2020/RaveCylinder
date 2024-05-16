#include "WLED.h"
#include "WLED_FX.h"
#include "WLED_fcn_declare.h"

namespace ravecylinder {

// uint16_t rand16seed = 0;
time_t localTime = time(nullptr);
bool stateChanged = false;
uint16_t ledMaps = 0xFFFF;

// Global variable extern initialization

// LED CONFIG
bool autoSegments = true;
byte bootPreset = 0; // save preset to load after power-up
bool turnOnAtBoot = true;
unsigned long lastEditTime = 0;
bool doSerializeConfig = false;
bool doReboot = false;
bool correctWB = false;
bool doInitBusses = false;

// SYNC CONFIG
bool notifyDirect = false; // send notification if change via UI or HTTP API

// color
byte lastRandomIndex = 0;
bool gammaCorrectCol = true;
float gammaCorrectVal = 2.8;

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
byte nightlightTargetBri = 0; // brightness after nightlight is over
byte nightlightDelayMins = 60;
byte nightlightMode =
    NL_MODE_FADE; // See const.h for available modes. Was nightlightFade
bool nightlightActive = false;
bool nightlightActiveOld = false;
uint32_t nightlightDelayMs = 10;
byte nightlightDelayMinsDefault = nightlightDelayMins;
unsigned long nightlightStartTime;
byte briNlT = 0;                // current nightlight brightness
byte *colNlT[4] = {0, 0, 0, 0}; // current nightlight color

// brightness
byte briS = 128;
byte bri = briS;
byte briLast = 128;
byte briMultiplier = 100;

// playlists
int16_t currentPlaylist = -1;

// network
UDPClient udpClient;
DDPOutput ddpOutput;
// presets
byte currentPreset = 0;
byte errorFlag = 0;

// led fx library object
char *ledmapNames[WLED_MAX_LEDMAPS] = {nullptr};

// Temp buffer
char *obuf = nullptr;
uint16_t olen = 0;

BusConfig busConfig;
BusManager busses;
WLEDFileSystem WLED_FS;
Usermods usermods;

// Currently 4 types defined, to be fine tuned and new types added
typedef enum UM_SoundSimulations {
  UMS_BeatSin = 10,
  UMS_WeWillRockYou = 0,
  UMS_10_3,
  UMS_14_3
} um_soundSimulations_t;
um_data_t *simulateSound(uint8_t simulationId) {
  static uint8_t samplePeak;
  static float FFT_MajorPeak;
  static uint8_t maxVol;
  static uint8_t binNum;

  static float volumeSmth;
  static uint16_t volumeRaw;
  static float my_magnitude;

  // arrays
  uint8_t *fftResult;

  static um_data_t *um_data = nullptr;

  if (!um_data) {
    // claim storage for arrays
    fftResult = (uint8_t *)malloc(sizeof(uint8_t) * 16);

    // initialize um_data pointer structure
    // NOTE!!!
    // This may change as AudioReactive usermod may change
    um_data = new um_data_t;
    um_data->u_size = 8;
    um_data->u_type = new um_types_t[um_data->u_size];
    um_data->u_data = new void *[um_data->u_size];
    um_data->u_data[0] = &volumeSmth;
    um_data->u_data[1] = &volumeRaw;
    um_data->u_data[2] = fftResult;
    um_data->u_data[3] = &samplePeak;
    um_data->u_data[4] = &FFT_MajorPeak;
    um_data->u_data[5] = &my_magnitude;
    um_data->u_data[6] = &maxVol;
    um_data->u_data[7] = &binNum;
  } else {
    // get arrays from um_data
    fftResult = (uint8_t *)um_data->u_data[2];
  }

  uint32_t ms = millis();

  switch (simulationId) {
  default:
  case UMS_BeatSin:
    for (int i = 0; i < 16; i++)
      fftResult[i] = beatsin8(120 / (i + 1), 0, 255);
    // fftResult[i] = (beatsin8(120, 0, 255) + (256/16 * i)) % 256;
    volumeSmth = fftResult[8];
    break;
  case UMS_WeWillRockYou:
    if (ms % 2000 < 200) {
      volumeSmth = random8(255);
      for (int i = 0; i < 5; i++)
        fftResult[i] = random8(255);
    } else if (ms % 2000 < 400) {
      volumeSmth = 0;
      for (int i = 0; i < 16; i++)
        fftResult[i] = 0;
    } else if (ms % 2000 < 600) {
      volumeSmth = random8(255);
      for (int i = 5; i < 11; i++)
        fftResult[i] = random8(255);
    } else if (ms % 2000 < 800) {
      volumeSmth = 0;
      for (int i = 0; i < 16; i++)
        fftResult[i] = 0;
    } else if (ms % 2000 < 1000) {
      volumeSmth = random8(255);
      for (int i = 11; i < 16; i++)
        fftResult[i] = random8(255);
    } else {
      volumeSmth = 0;
      for (int i = 0; i < 16; i++)
        fftResult[i] = 0;
    }
    break;
  case UMS_10_3:
    for (int i = 0; i < 16; i++)
      fftResult[i] =
          inoise8(beatsin8(90 / (i + 1), 0, 200) * 15 + (ms >> 10), ms >> 3);
    volumeSmth = fftResult[8];
    break;
  case UMS_14_3:
    for (int i = 0; i < 16; i++)
      fftResult[i] =
          inoise8(beatsin8(120 / (i + 1), 10, 30) * 10 + (ms >> 14), ms >> 3);
    volumeSmth = fftResult[8];
    break;
  }

  samplePeak = random8() > 250;
  FFT_MajorPeak = volumeSmth;
  maxVol = 10; // this gets feedback fro UI
  binNum = 8;  // this gets feedback fro UI
  volumeRaw = volumeSmth;
  my_magnitude =
      10000.0 / 8.0f; // no idea if 10000 is a good value for FFT_Magnitude ???
  if (volumeSmth < 1)
    my_magnitude = 0.001f; // noise gate closed - mute

  return um_data;
}
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

Bus::Bus(const uint16_t len) : _len(len), _pixels(nullptr) {
  if (!allocData(_len))
    return;
}

CRGB *Bus::allocData(size_t size) {
  if (_pixels)
    free(_pixels); // should not happen, but for safety
  return _pixels = (CRGB *)(size > 0 ? calloc(size, sizeof(CRGB)) : nullptr);
}

int Bus::getLength() { return _len; }

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

void Rave::setup() { beginStrip(); }

void Rave::loop() {
  strip().service();
  if (doInitBusses) {
    doInitBusses = false;

    busses.bus = Bus(busConfig._len);
    bool aligned =
        strip()
            .checkSegmentAlignment(); // see if old segments match old bus(ses)
    // busses.removeAll();
    // uint32_t mem = 0, globalBufMem = 0;
    // uint16_t maxlen = 0;
    // for (uint8_t i = 0; i < WLED_MAX_BUSSES+WLED_MIN_VIRTUAL_BUSSES; i++) {
    //   if (busConfigs[i] == nullptr) break;
    //   mem += BusManager::memUsage(*busConfigs[i]);
    //   if (useGlobalLedBuffer && busConfigs[i]->start + busConfigs[i]->count >
    //   maxlen) {
    //       maxlen = busConfigs[i]->start + busConfigs[i]->count;
    //       globalBufMem = maxlen * 4;
    //   }
    //   if (mem + globalBufMem <= MAX_LED_MEMORY) {
    //     busses.add(*busConfigs[i]);
    //   }
    //   delete busConfigs[i]; busConfigs[i] = nullptr;
    // }
    strip().finalizeInit(); // also loads default ledmap if present
    if (aligned)
      strip().makeAutoSegments();
    else
      strip().fixInvalidSegments();
    doSerializeConfig = true;
  }
}

void Rave::beginStrip() {
  // Initialize NeoPixel Strip and button
  strip().finalizeInit(); // busses created during deserializeConfig()
  strip().makeAutoSegments();
  strip().setBrightness(0);
  // strip().setShowCallback(handleOverlayDraw);
  if (turnOnAtBoot) {
    if (briS > 0)
      bri = briS;
    else if (bri == 0)
      bri = 128;
  } else {
    // fix for #3196
    briLast = briS;
    bri = 0;
    strip().fill(BLACK);
    strip().show();
  }
  //   if (bootPreset > 0) {
  //     applyPreset(bootPreset, CALL_MODE_INIT);
  //   }
  // colorUpdated(CALL_MODE_INIT);

  // init relay pin
  //   if (rlyPin>=0)
  //     digitalWrite(rlyPin, (rlyMde ? bri : !bri));
}

} // namespace ravecylinder