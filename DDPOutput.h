// DDPOutput handles generating the Distrubuted
// Display Protocol (DDP) packets that will be sent
// to the DDP UDP Server (FPP in this case).
// DDP Info: http://www.3waylabs.com/ddp/
// FPP Info: https://github.com/FalconChristmas/fpp

#include <cstdint>
#include <vector>

#include "Pixel.h"

namespace ravecylinder {

struct Packet {
  std::vector<uint8_t> header;
  std::vector<uint8_t> pixel_data;
  std::vector<uint8_t> GetBytes() {
    std::vector rtrn = header;
    rtrn.insert(rtrn.end(), pixel_data.begin(), pixel_data.end());
    return rtrn;
  }
};

class DDPOutput {
public:
  DDPOutput();
  ~DDPOutput() {}

  std::vector<Packet> GenerateFrame(const CRGB *pixels, int num_pixels);

private:
  void SetDDPPacketHeader(std::vector<uint8_t> *packet, uint32_t offset,
                          uint16_t num_pixels_in_packet, bool push_frame);
};

} // namespace ravecylinder