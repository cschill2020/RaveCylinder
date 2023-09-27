#include "DDPOutput.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "UDPClient.h"
#include <chrono>
#include <iostream>
#include <thread>

#define DDP_PORT "4048"

using namespace ravecylinder;

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinder.local", DDP_PORT);

  CRGB pixels[NUM_PIXELS];
  while (true) {
    pixels[0] = CRGB(CRGB::Red);
    pixels[1] = CRGB(CRGB::Green);
    pixels[2] = CRGB(CRGB::Blue);

    DDPOutput output1;
    output1.GenerateFrame(pixels, 0);
    client.SendTo(output1.GetBytes());
    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    pixels[0] = CRGB(CRGB::Black);
    pixels[1] = CRGB(CRGB::Black);
    pixels[2] = CRGB(CRGB::Black);

    DDPOutput output2;
    output2.GenerateFrame(pixels, 0);
    client.SendTo(output2.GetBytes());
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
  }
}