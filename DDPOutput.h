// DDPOutput handles generating the Distrubuted
// Display Protocol (DDP) packets that will be sent
// to the DDP UDP Server (FPP in this case).
// DDP Info: http://www.3waylabs.com/ddp/
// FPP Info: https://github.com/FalconChristmas/fpp

#include <cstdint>
#include <vector>

#include "Pixel.h"

namespace ravecylinder {

#define NUM_PIXELS 3

class DDPOutput {
public:
  DDPOutput();
  ~DDPOutput() {}

  void GenerateFrame(const CRGB* pixels, uint32_t offset);
  std::vector<uint8_t> GetBytes();

private:
  std::vector<uint8_t> packet_;
  std::vector<uint8_t> CreateTestDDPHeader(uint32_t offset);
};

}