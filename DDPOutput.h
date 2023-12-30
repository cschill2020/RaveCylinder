// DDPOutput handles generating the Distrubuted
// Display Protocol (DDP) packets that will be sent
// to the DDP UDP Server (FPP in this case).
// DDP Info: http://www.3waylabs.com/ddp/
// FPP Info: https://github.com/FalconChristmas/fpp

#include <cstdint>
#include <vector>

#include "Pixel.h"

namespace ravecylinder {

// Packet contain the header and pixel color information.
struct Packet {
  // 10-byte DDP header
  std::vector<uint8_t> header;
  // Max length 480*3 = 1440 bytes defining each pixel in the packet.
  std::vector<uint8_t> pixel_data;

  // Combine the header and pixel_data.
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

  // Convert the pixel matrix defining a single frame into DDP packets
  // for the controller.
  std::vector<Packet> GenerateFrame(const CRGB *pixels, int num_pixels);

private:
  // Create the header.  Since each packet stores max 480 pixel instructions,
  // the header keeps track of the *offset* and *num_pixels_in_packet*.
  // The push_frame instruction is set for the last packet of the frame, 
  // which tells the controller to display the frame.
  void SetDDPPacketHeader(std::vector<uint8_t> *packet, uint32_t offset,
                          uint16_t num_pixels_in_packet, bool push_frame);
};

} // namespace ravecylinder