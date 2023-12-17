#include "ColorUtils.h"
#include "DDPOutput.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "UDPClient.h"
#include <iostream>
#include <thread>

#define DDP_PORT "4048"
#define NUM_PIXELS 100

using namespace std::literals;

using namespace ravecylinder;

CRGB pixels[NUM_PIXELS];

uint8_t pattern_counter = 0;
void nextPattern() {
  std::cout << "pattern_counter: " << unsigned(pattern_counter) << std::endl;
  pattern_counter = (pattern_counter + 1) % 3;
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

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);

  while (true) {
    switch (pattern_counter) {
    case 0:
      testSin8();
      break;
    case 1:
      rainbowBeat();
      break;
    case 2:
      sinelon();
      break;
    }
    EVERY_N_MILLIS(20) { ++gHue; }
    EVERY_N_SECONDS(5) { nextPattern(); }

    // TDOD: Separate the frame generation and display into parallel threads
    // using a TaskScheduler.
    DDPOutput output;
    std::vector<Packet> packets = output.GenerateFrame(pixels, NUM_PIXELS);
    for (auto &packet : packets) {
      client.SendTo(packet.GetBytes());
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}