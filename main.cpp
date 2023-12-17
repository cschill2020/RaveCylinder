#include "ColorPalettes.h"
#include "ColorUtils.h"
#include "DDPOutput.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "UDPClient.h"
#include <iostream>
#include <thread>

#define DDP_PORT "4048"
#define NUM_PIXELS 20

using namespace std::literals;

using namespace ravecylinder;

CRGB pixels[NUM_PIXELS];

CRGBPalette16 currentPalette;
TBlendType currentBlending; // NOBLEND or LINEARBLEND

uint8_t pattern_counter = 0;
void nextPattern() {
  std::cout << "pattern_counter: " << unsigned(pattern_counter) << std::endl;
  pattern_counter = (pattern_counter + 1) % 5;
}

//------- Test sequences below -------//
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

void sinelon(uint8_t hue) {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(pixels, NUM_PIXELS, 10);
  int pos = beatsin16(30, 0, NUM_PIXELS - 1);
  pixels[pos] += CHSV(hue, 255, 192);
}

void sinelonWithPalette(uint8_t color_index) {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(pixels, NUM_PIXELS, 10);
  int pos = beatsin16(30, 0, NUM_PIXELS - 1);
  pixels[pos] = ColorFromPalette(currentPalette, color_index, 100,
                                         currentBlending);
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

const CRGBPalette16 myRedWhiteBluePalette = {
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
    pixels[i] = color;
    colorIndex += 3;
  }
}

int main() {
  // Setup the connection to the controller.  The controller is 
  // receiving simple UDP packets on DDP_PORT.  It is expecting
  // packets formatted according to the ddp protocol defined:
  // http://www.3waylabs.com/ddp/
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);

  // This is basically loop() in arduino code.  Implement 
  // a few test sequences to cycle through.  Shows off some
  // timing macros like EVERY_N_ and cycling through color 
  // palettes.
  // Each cycle of the loop fills the global CRGB matrix 
  // with colors for NUM_PIXELS.  Each frame is then sent
  // to the controller using DDP/UDP push.
  while (true) {
    ChangePalettePeriodically();
    switch (pattern_counter) {
    case 0:
      EVERY_N_MILLIS(20) {
        static uint8_t ghue = 0;
        sinelon(ghue++);
      }
      break;
    case 1:
      EVERY_N_MILLIS(20) {
        static uint8_t color_index = 0;
        sinelonWithPalette(color_index++);
      }
      break;
    case 2:
      rainbowBeat();
      break;
    case 3:
      testSin8();
      break;
    case 4:
      EVERY_N_MILLIS(10) {
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */
        FillLEDsFromPaletteColors(startIndex);
      }
      break;
    }
    EVERY_N_SECONDS(5) { nextPattern(); }

    // TDOD: Separate the frame generation and display into parallel threads
    // using a TaskScheduler.
    // DDPOutput converts the pixel matrix into the set of packets processed 
    // by the controller.
    DDPOutput output;
    std::vector<Packet> packets = output.GenerateFrame(pixels, NUM_PIXELS);
    for (auto &packet : packets) {
      // Send the packets.
      client.SendTo(packet.GetBytes());
    }
    // A short delay is helpful to ensure the controller doesn't get 
    // overwhelmed with packets.
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}