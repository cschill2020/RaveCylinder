#include "WLED_FX.h"
#include "WLED_fcn_declare.h"
#include "WLED.h"

namespace ravecylinder {

//uint16_t rand16seed = 0;
time_t localTime = time(nullptr);
uint8_t randomPaletteChangeTime = 5;
bool stateChanged = false;
byte lastRandomIndex = 0;
char* ledmapNames[WLED_MAX_LEDMAPS] = { nullptr };
uint16_t ledMaps = 0xFFFF;
CRGB _pixels[NUM_PIXELS];

BusManager busses;
WLEDFileSystem WLED_FS;

int16_t extractModeDefaults(uint8_t mode, const char* segVar) {
    if (mode < strip().getModeCount()) {
        char lineBuffer[128] = "";
        strncpy(lineBuffer, strip().getModeData(mode), 127);
        lineBuffer[127] = '\0'; // terminate string
        if (lineBuffer[0] != 0) {
            char* startPtr = strrchr(lineBuffer, ';'); // last ";" in FX data
            if (!startPtr)
                return -1;

            char* stopPtr = strstr(startPtr, segVar);
            if (!stopPtr)
                return -1;

            stopPtr += strlen(segVar) + 1; // skip "="
            return atoi(stopPtr);
        }
    }
    return -1;
}

void WS2812FX::loadCustomPalettes() {
}

int Bus::getLength() {
return NUM_PIXELS;
}
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
}