// DDPOutput handles generating the Distrubuted
// Display Protocol (DDP) packets that will be sent
// to the DDP UDP Server (FPP in this case).
// DDP Info: http://www.3waylabs.com/ddp/
// FPP Info: https://github.com/FalconChristmas/fpp

#include <cstdint>
#include <vector>

#define NUM_PIXELS 3

// What if I have 32 or 8 byte pixels?
struct Pixel {
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

struct DenseFrame {
  Pixel pixel[NUM_PIXELS];
};

class DDPOutput {
public:
  DDPOutput();
  ~DDPOutput() {}

  void GenerateFrame(const DenseFrame &frame, uint32_t offset);
  std::vector<uint8_t> GetBytes();

private:
  std::vector<uint8_t> packet_;
  std::vector<uint8_t> CreateTestDDPHeader(uint32_t offset);
};