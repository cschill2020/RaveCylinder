#include "ColorPalettes.h"
#include "ColorUtils.h"
#include "Pixel.h"
#include "PixelUtils.h"
#include "WLED.h"
#include "WLED_FX.h"

#include <chrono>
#include <httpserver.hpp>
#include <iostream>
#include <thread>
// #include "json.hpp"

#define DDP_PORT "4048"

using namespace std::literals;
using namespace ravecylinder;
using namespace httpserver;
using namespace std::chrono;

int startHTTPServer() {
  webserver ws = create_webserver(8088);

  index_response index;
  json_response json;
  settings_response settings;

  ws.register_resource("/", &index, true);
  ws.register_resource("/json", &json, true);
  ws.register_resource("/settings", &settings, true);

  ws.start(true);

  return 0;
}

// void initStrip(int width, int height, int map, int b, uint8_t mode, uint8_t
// s,
//                uint8_t i, uint8_t pal, uint32_t c1, uint32_t c2, uint32_t c3,
//                uint8_t custom1, uint8_t custom2, uint8_t custom3, int check1,
//                int check2, int check3, const std::string &text) {
//   /*WS2812FX(), model(m), mapping(map), brightness(b)*/
//   if (width > 1 && height > 1) {
//     strip().isMatrix = true;
//   }
//   ravecylinder::WS2812FX::Panel p;
//   if (map == 0 || map == 2) {
//     strip().appendSegment(Segment(0, width, 0, height));
//     p.width = width;
//     p.height = height;
//   } else {
//     strip().appendSegment(Segment(0, height, 0, width));
//     p.width = height;
//     p.height = width;
//   }
//   strip().milliampsPerLed = 0; // need to turn off the power calculation
//   strip()._segments[0].transitional = false;
//   strip().setMode(0, mode);
//   strip()._segments[0].speed = s;
//   strip()._segments[0].intensity = i;
//   strip()._segments[0].palette = pal;
//   strip()._segments[0].custom1 = custom1;
//   strip()._segments[0].custom2 = custom2;
//   strip()._segments[0].custom3 = custom3;
//   strip()._segments[0].check1 = check1;
//   strip()._segments[0].check2 = check2;
//   strip()._segments[0].check3 = check3;
//   // if (text != "") {
//   //     _segments[0].name = new char[text.length() + 1];
//   //     strncpy(_segments[0].name, text.c_str(), text.length() + 1);
//   // }
//   strip()._segments[0].refreshLightCapabilities();

//   strip().setColor(0, c1);
//   strip().setColor(1, c2);
//   strip().setColor(2, c3);

//   strip().panel.push_back(p);
//   // pushCurrent(this);
//   strip().finalizeInit();
//   // popCurrent();
// }

int main() {

  // Setup the connection to the controller.  The controller is
  // receiving simple UDP packets on DDP_PORT.  It is expecting
  // packets formatted according to the ddp protocol defined:
  // http://www.3waylabs.com/ddp/
  // UDPClient client;
  if (enableUDP) {
    udpClient.OpenConnection("ravecylinder.local", DDP_PORT);
  }
  
  // Initiate strip
  // initStrip(/*width*/ NUM_PIXELS, /*height*/ 1, /*map*/ 0, /*brightness*/ 15,
  //           /*fx mode*/ 10, /*speed*/ 128, /*intensity*/ 255, /*palette*/ 11,
  //           CRGB::Red, CRGB::Blue, CRGB::Blue, 0, 0, 0, 0, 0, 0, "");
  // Initiate web server

  std::thread server_thread(startHTTPServer);
  server_thread.detach();

  // Very basic setup of the strip.  Could move the udpClient and
  // startHTTPServer code here.
  ravecylinder::Rave::instance().setup();

  // This is basically loop() in arduino code.
  while (true) {
    ravecylinder::Rave::instance().loop();
  }
}