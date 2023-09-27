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
#define DDP_MAX_DATALEN (480 * 3) // 1440 fits nicely in an ethernet packet
#define DDP_PACKET_LEN (DDP_HEADER_LEN + DDP_MAX_DATALEN)

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

std::vector<uint8_t> DDPOutput::CreateTestDDPHeader(uint32_t offset) {
  /* Header will be 10 bytes */
  std::vector<uint8_t> header;
  // Byte 0: Config Flags
  header.push_back(DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH);
  // Byte 1: Sequnece #
  header.push_back(0x00);
  // Byte 2: I do not think this is used by FPP.
  header.push_back(0x00);
  // Byte 3: Device ID of Display.  Not sure how it is
  // used by FPP yet, so keeping it default.
  header.push_back(DDP_ID_DISPLAY);
  // Offset: DDP Backend expects BigEndian.
  header.push_back((offset & 0xFF000000) >> 24);
  header.push_back((offset & 0xFF0000) >> 16);
  header.push_back((offset & 0xFF00) >> 8);
  header.push_back((offset & 0xFF));
  // Test data has length 9 (3 channels * 3 RGB), 27 pixels.  I think...
  header.push_back((NUM_PIXELS*3 & 0xFF00) >> 8);
  header.push_back(NUM_PIXELS*3 & 0xFF);

  return header;
}

void DDPOutput::GenerateFrame(const CRGB* pixels, uint32_t offset) {
  std::vector<uint8_t> header = CreateTestDDPHeader(offset);
  packet_.insert(packet_.end(), header.begin(), header.end());
  for (int i = 0; i < NUM_PIXELS; ++i) {
    packet_.push_back(pixels[i].red);
    packet_.push_back(pixels[i].green);
    packet_.push_back(pixels[i].blue);
  }
}

std::vector<uint8_t> DDPOutput::GetBytes() {
  return packet_;
}
}