#include "ColorUtils.h"
#include "DDPOutput.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "UDPClient.h"
#include <chrono>
#include <iostream>
#include <thread>

#define DDP_PORT "4048"

using namespace std::literals;
using clock_type = std::chrono::high_resolution_clock;

using namespace ravecylinder;

CRGB pixels[NUM_PIXELS];

void Blink(const UDPClient &udp_client) {
  pixels[0] = CRGB(CRGB::Red);
  pixels[1] = CRGB(CRGB::Green);
  pixels[2] = CRGB(CRGB::Blue);

  DDPOutput output1;
  output1.GenerateFrame(pixels, 0);
  udp_client.SendTo(output1.GetBytes());
  std::this_thread::sleep_for(std::chrono::milliseconds(40));

  pixels[0] = CRGB(CRGB::Black);
  pixels[1] = CRGB(CRGB::Black);
  pixels[2] = CRGB(CRGB::Black);

  DDPOutput output2;
  output2.GenerateFrame(pixels, 0);
  udp_client.SendTo(output2.GetBytes());
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

void TestHue(const UDPClient &udp_client, uint8_t hue) {
  for (int i = 0; i < NUM_PIXELS; ++i) {
    pixels[i] = CHSV(hue + (i * 10), 255, 255);
    DDPOutput output2;
    output2.GenerateFrame(pixels, 0);
    udp_client.SendTo(output2.GetBytes());
  }
}

uint8_t pattern_counter = 0;
void nextPattern() { pattern_counter = (pattern_counter + 1) % 3; }

//------- Put your patterns below -------//

void movingDots() {

  uint16_t posBeat = beatsin16(30, 0, NUM_PIXELS - 1, 0, 0);
  uint16_t posBeat2 = beatsin16(60, 0, NUM_PIXELS - 1, 0, 0);

  uint16_t posBeat3 = beatsin16(30, 0, NUM_PIXELS - 1, 0, 32767);
  uint16_t posBeat4 = beatsin16(60, 0, NUM_PIXELS - 1, 0, 32767);

  // Wave for LED color
  uint8_t colBeat = beatsin8(45, 0, 255, 0, 0);

  pixels[(posBeat + posBeat2) / 2] = CHSV(colBeat, 255, 255);
  pixels[(posBeat3 + posBeat4) / 2] = CHSV(colBeat, 255, 255);

  fadeToBlackBy(pixels, NUM_PIXELS, 10);
}

void rainbowBeat() {

  uint16_t beatA = beatsin16(30, 0, 255);
  uint16_t beatB = beatsin16(20, 0, 255);
  fill_rainbow(pixels, NUM_PIXELS, (beatA + beatB) / 2, 8);
}

void redWhiteBlue() {

  uint16_t sinBeat = beatsin16(30, 0, NUM_PIXELS - 1, 0, 0);
  uint16_t sinBeat2 = beatsin16(30, 0, NUM_PIXELS - 1, 0, 21845);
  uint16_t sinBeat3 = beatsin16(30, 0, NUM_PIXELS - 1, 0, 43690);

  pixels[sinBeat] = CRGB::Blue;
  pixels[sinBeat2] = CRGB::Red;
  pixels[sinBeat3] = CRGB::White;

  fadeToBlackBy(pixels, NUM_PIXELS, 10);
}

void testSin8() {
  uint16_t sinBeat = beatsin16(30, 0, NUM_PIXELS - 1, 0, 0);
  pixels[sinBeat] = CRGB::Blue;
  fadeToBlackBy(pixels, NUM_PIXELS, 100);
}

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);

  auto delay_time = 5s;
  auto target_time = clock_type::now() + delay_time;
  int nloops = 0;
  while (nloops++ < 1000) {
    switch (pattern_counter) {
    case 0:
      testSin8();
      //movingDots();
      break;
    case 1:
      rainbowBeat();
      break;
    case 2:
      redWhiteBlue();
      break;
    }

    //if (clock_type::now() > (target_time)) {
    //nextPattern();
    //target_time += delay_time;
    //}
    // FastLED.show();
    DDPOutput output2;
    output2.GenerateFrame(pixels, 0);
    client.SendTo(output2.GetBytes());
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
  }
}