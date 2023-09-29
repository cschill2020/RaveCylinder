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

void Blink(const UDPClient &udp_client) {
  CRGB pixels[NUM_PIXELS];
  while (true) {
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
}

void TestHue(const UDPClient &udp_client, CRGB *pixels, uint8_t hue) {
  for (int i = 0; i < NUM_PIXELS; ++i) {
    pixels[i] = CHSV(hue + (i*10), 255, 255);
    DDPOutput output2;
    output2.GenerateFrame(pixels, 0);
    udp_client.SendTo(output2.GetBytes());
  }
}

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);
  auto delay_time = 25ms;

  CRGB pixels[NUM_PIXELS];
  uint8_t hue = 0;
  auto when_started = clock_type::now();
  auto target_time = when_started + delay_time;
  while (true) {
    TestHue(client, pixels, hue);
    std::this_thread::sleep_until(target_time);
    hue++;
    target_time += delay_time;
  }
}