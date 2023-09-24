#include "UDPClient.h"

// #define DDP_PORT "4048"
#define DDP_PORT "2222"

// FOR DDP Proto
#define MAXLINE 1024

#define DDP_HEADER_LEN (sizeof(struct ddp_hdr_struct))
#define DDP_MAX_DATALEN (480 * 3) // fits nicely in an ethernet packet

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

// DDP header format
// header is 10 bytes (14 if TIME flag used)
struct DDPHeader {
  uint8_t flags1;
  uint8_t flags2;
  uint8_t type;
  uint8_t id;
  uint32_t offset;
  uint16_t length; // MSB
};
// for example code below:
// struct ddp_hdr_struct dh;    // header storage
// unsigned char *databuf; // pointer to data buffer

int main() {
  UDPClient client;
  client.OpenConnection("ravecylinderpi.local", DDP_PORT);
  client.SendTo("Testing");
  client.SendTo("StillUp?");
}