#include "DDPOutput.h"
#include "UDPClient.h"

#include <chrono>
#include <iostream>
#include <thread>

#define DDP_PORT "4048"

Pixel CreateTestPixel24bit(uint8_t R, uint8_t G, uint8_t B) {
  return {R, G, B};
}

int main() {
  UDPClient client;
  client.OpenConnection("192.168.86.46", DDP_PORT);

  DenseFrame frame;
  frame.pixel[0] = CreateTestPixel24bit(255, 0, 0);
  frame.pixel[1] = CreateTestPixel24bit(0, 255, 0);
  frame.pixel[2] = CreateTestPixel24bit(0, 0, 255);

  DDPOutput output;
  output.GenerateFrame(frame, 0);
  client.SendTo(output.GetBytes());
}