// DDPOutput handles generating the Distrubuted
// Display Protocol (DDP) packets that will be sent
// to the DDP UDP Server (FPP in this case).
// DDP Info: http://www.3waylabs.com/ddp/
// FPP Info: https://github.com/FalconChristmas/fpp

#include "DDPOutput.h"
#include <cstdint>
#include <iostream>
#include <string>

namespace ravecylinder {

#define DDP_HEADER_LEN 10
#define DDP_MAX_PIXELS_PER_PACKET 480
#define DDP_MAX_DATALEN                                                        \
  (DDP_MAX_PIXELS_PER_PACKET * 3) // 1440 fits nicely in an ethernet packet
#define DDP_PACKET_LEN (DDP_HEADER_LEN + DDP_MAX_DATALEN)
// For WS2811 this is 3
#define NUM_LED_PER_PIXEL 3
// Used as bit mask for the DDP header config byte 0.
// For this we only need VER1 and PUSH.  They are
// defined for posterity.
#define DDP_FLAGS1_VER 0xc0  // version mask
#define DDP_FLAGS1_VER1 0x40 // version=1
#define DDP_FLAGS1_PUSH 0x01
#define DDP_FLAGS1_QUERY 0x02
#define DDP_FLAGS1_REPLY 0x04
#define DDP_FLAGS1_STORAGE 0x08
#define DDP_FLAGS1_TIME 0x10

#define DDP_ID_DISPLAY 1
#define DDP_ID_CONFIG 250
#define DDP_ID_STATUS 251

DDPOutput::DDPOutput() {}

// 1: Only set PUSH if it is the last packet for the frame.
// 2: Apply offset when there are more pixels than available packet size.
void DDPOutput::SetDDPPacketHeader(std::vector<uint8_t> *packet,
                                   uint32_t offset,
                                   uint16_t num_pixels_in_packet,
                                   bool push_frame = false) {
  std::cout << "uoffset = " << offset << std::endl;
  std::cout << "ulen = " << num_pixels_in_packet << std::endl;
  // DDP Header is 10 bytes
  // Byte 0: Config Flags
  if (push_frame) {
    packet->push_back(DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH);
  } else {
    packet->push_back(DDP_FLAGS1_VER1);
  }
  // Byte 1: Sequnece #
  packet->push_back(0x00);
  // Byte 2: I do not think this is used by FPP.
  packet->push_back(0x00);
  // Byte 3: Device ID of Display.  Not sure how it is
  // used by FPP yet, so keeping it default.
  packet->push_back(DDP_ID_DISPLAY);
  // Offset: DDP Backend expects BigEndian.
  packet->push_back((offset & 0xFF000000) >> 24);
  packet->push_back((offset & 0xFF0000) >> 16);
  packet->push_back((offset & 0xFF00) >> 8);
  packet->push_back((offset & 0xFF));
  // Test data has length 9 (3 channels * 3 RGB), 27 pixels.  I think...
  packet->push_back(((num_pixels_in_packet * NUM_LED_PER_PIXEL) & 0xFF00) >> 8);
  packet->push_back((num_pixels_in_packet * NUM_LED_PER_PIXEL) & 0xFF);
}

std::vector<Packet> DDPOutput::GenerateFrame(const CRGB *pixels) {
  // Number of pixels in the last packet;
  uint16_t rem_pixels = NUM_PIXELS % DDP_MAX_PIXELS_PER_PACKET;
  // Total number of packets;
  uint16_t tot_packets = NUM_PIXELS / DDP_MAX_PIXELS_PER_PACKET;
  if (rem_pixels != 0) {
    tot_packets += 1;
  }
  std::vector<Packet> packets;
  int packet_ctr = 0;
  for (int i = 0; i < NUM_PIXELS; ++i) {
    // Create a new packet.
    if (i % DDP_MAX_PIXELS_PER_PACKET == 0) {
      Packet packet;
      bool push = false;
      uint16_t num_pixels_in_packet = DDP_MAX_PIXELS_PER_PACKET;
      // Push if last packet contains all the LEDs
      if (packet_ctr == tot_packets - 1) {
        num_pixels_in_packet = rem_pixels;
        push = true;
      }
      uint32_t offset = packet_ctr * DDP_MAX_DATALEN;
      SetDDPPacketHeader(&packet.header, offset, num_pixels_in_packet, push);
      packets.push_back(packet);
      packet_ctr++;
    }
    packets[packet_ctr - 1].pixel_data.push_back(pixels[i].red);
    packets[packet_ctr - 1].pixel_data.push_back(pixels[i].green);
    packets[packet_ctr - 1].pixel_data.push_back(pixels[i].blue);
  }
  return packets;
}

} // namespace ravecylinder