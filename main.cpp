#include "ColorPalettes.h"
#include "ColorUtils.h"
#include "DDPOutput.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "UDPClient.h"
#include <iostream>
#include <thread>

#define DDP_PORT "4048"
#define NUM_PIXELS 10

using namespace std::literals;

using namespace ravecylinder;

CRGB pixels[NUM_PIXELS];

uint8_t pattern_counter = 3;
void nextPattern() {
  std::cout << "pattern_counter: " << unsigned(pattern_counter) << std::endl;
  pattern_counter = (pattern_counter + 1) % 4;
}

//------- Put your patterns below -------//
void rainbowBeat() {

  uint16_t beatA = beatsin16(30, 0, 255);
  uint16_t beatB = beatsin16(20, 0, 255);
  fill_rainbow(pixels, NUM_PIXELS, (beatA + beatB) / 2, 8);
}

void testSin8() {
  uint16_t pos1 = beatsin16(20, 0, NUM_PIXELS - 1, 0, 0);
  uint16_t pos2 = beatsin16(20, 0, NUM_PIXELS - 1, 0, 32767);
  pixels[pos1] += CRGB::Blue;
  pixels[pos2] += CRGB::Red;
  fadeToBlackBy(pixels, NUM_PIXELS, 10);
}

uint8_t gHue = 0;
void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(pixels, NUM_PIXELS, 10);
  int pos = beatsin16(13, 0, NUM_PIXELS - 1);
  pixels[pos] += CHSV(gHue, 255, 192);
}

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType currentBlending; // NOBLEND or LINEARBLEND
int gHue2 = 0;
void sinelon2() { // a colored dot sweeping back and forth, with fading trails

  // fadeToBlackBy(pixels, NUM_PIXELS, 32);
  int pos1 = beatsin16(23, 0, NUM_PIXELS - 1);
  int pos2 = beatsin16(28, 0, NUM_PIXELS - 1);

  pixels[(pos1 + pos2) / 2] +=
      ColorFromPalette(currentPalette, gHue2++, 255, currentBlending);

} // sinelon()

void sinelon2Loop() {
  EVERY_N_MILLIS(100) {
    uint8_t maxChanges = 24;
    nblendPaletteTowardPalette(
        currentPalette, targetPalette,
        maxChanges); // AWESOME palette blending capability.
  }

  EVERY_N_SECONDS(
      5) { // Change the target palette to a random one every 5 seconds.
    static uint8_t baseC =
        random8(); // You can use this as a baseline colour if you want
                   // similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)),
                                  CHSV(random8(), 255, random8(128, 255)),
                                  CHSV(random8(), 192, random8(128, 255)),
                                  CHSV(random8(), 255, random8(128, 255)));
  }
  EVERY_N_MILLIS(50) { // FastLED based non-blocking delay to update/display
    fadeToBlackBy(pixels, NUM_PIXELS, 32);
  }           // the sequence.
  sinelon2(); // Call our sequence.
              //}
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette() {
  for (int i = 0; i < 16; ++i) {
    currentPalette[i] = CHSV(random8(), 255, random8());
  }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette() {
  // 'black out' all 16 palette entries...
  fill_solid(currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette() {
  CRGB purple = CHSV(CHSV::HUE_PURPLE, 255, 255);
  CRGB green = CHSV(CHSV::HUE_GREEN, 255, 255);
  CRGB black = CRGB::Black;

  currentPalette =
      CRGBPalette16(green, green, black, black, purple, purple, black, black,
                    green, green, black, black, purple, purple, black, black);
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette = {
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue, CRGB::Black,

    CRGB::Red,  CRGB::Gray,  CRGB::Blue,  CRGB::Black,

    CRGB::Red,  CRGB::Red,   CRGB::Gray,  CRGB::Gray,
    CRGB::Blue, CRGB::Blue,  CRGB::Black, CRGB::Black};

void ChangePalettePeriodically() {
  uint8_t secondHand = (get_millisecond_timer() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond != secondHand) {
    lastSecond = secondHand;
    if (secondHand == 0) {
      currentPalette = RainbowColors;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 10) {
      currentPalette = RainbowStripeColors;
      currentBlending = NOBLEND;
    }
    if (secondHand == 15) {
      currentPalette = RainbowStripeColors;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 20) {
      SetupPurpleAndGreenPalette();
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 25) {
      SetupTotallyRandomPalette();
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 30) {
      SetupBlackAndWhiteStripedPalette();
      currentBlending = NOBLEND;
    }
    if (secondHand == 35) {
      SetupBlackAndWhiteStripedPalette();
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 40) {
      currentPalette = CloudColors;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 45) {
      currentPalette = PartyColors;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 50) {
      currentPalette = myRedWhiteBluePalette;
      currentBlending = NOBLEND;
    }
    if (secondHand == 55) {
      currentPalette = myRedWhiteBluePalette;
      currentBlending = LINEARBLEND;
    }
  }
}

void FillLEDsFromPaletteColors(uint8_t colorIndex) {
  uint8_t brightness = 255;

  for (int i = 0; i < NUM_PIXELS; ++i) {
    const auto &color = ColorFromPalette(currentPalette, colorIndex, brightness,
                                         currentBlending);
    std::cout << "index: " << i << std::endl;
    std::cout << "color r: " << unsigned(color.r) << std::endl;
    std::cout << "color g: " << unsigned(color.g) << std::endl;
    std::cout << "color b: " << unsigned(color.b) << std::endl;
    pixels[i] = color;
    colorIndex += 3;
  }
}

// void PaletteLoop() {
// }

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);

  // currentBlending = LINEARBLEND;
  //SetupPurpleAndGreenPalette();
  currentBlending = NOBLEND;
  while (true) {
    // ChangePalettePeriodically();
    switch (pattern_counter) {
    case 0:
      sinelon2Loop();
      break;
    case 1:
      rainbowBeat();
      break;
    case 2:
      testSin8();
      break;
    case 3:
      EVERY_N_MILLIS(10) {
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */
        //std::cout << unsigned(startIndex) << std::endl;
        FillLEDsFromPaletteColors(startIndex);
      }
    }
    // EVERY_N_MILLIS(20) { ++gHue; }
    // EVERY_N_SECONDS(5) { nextPattern(); }

    // TDOD: Separate the frame generation and display into parallel threads
    // using a TaskScheduler.
    DDPOutput output;
    std::vector<Packet> packets = output.GenerateFrame(pixels, NUM_PIXELS);
    for (auto &packet : packets) {
      client.SendTo(packet.GetBytes());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}