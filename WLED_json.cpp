#include "WLED.h"
#include "WLED_FX.h"
#include <httpserver.hpp>

namespace ravecylinder {
json serializeModeNames()
{
  char lineBuffer[256];
  json response = json::array();
  for (size_t i = 0; i < strip().getModeCount(); i++) {
    strncpy(lineBuffer, strip().getModeData(i), sizeof(lineBuffer)/sizeof(char)-1);
    lineBuffer[sizeof(lineBuffer)/sizeof(char)-1] = '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char* dataPtr = strchr(lineBuffer,'@');
      if (dataPtr) *dataPtr = 0; // terminate mode data after name
      response.insert(response.end(), lineBuffer);
    }
  }
  return response;
}

json serializeModeData()
{
  json response = json::array();
  char lineBuffer[256];
  for (size_t i = 0; i < strip().getModeCount(); i++) {
    strncpy(lineBuffer, strip().getModeData(i), sizeof(lineBuffer)/sizeof(char)-1);
    lineBuffer[sizeof(lineBuffer)/sizeof(char)-1] = '\0'; // terminate string
    if (lineBuffer[0] != 0) {
      char* dataPtr = strchr(lineBuffer,'@');
      if (dataPtr) {
        response.insert(response.end(),dataPtr+1);
      } else         response.insert(response.end(), "");
    }
  }
  return response;
}

std::shared_ptr<httpserver::http_response>
serveJson(const httpserver::http_request& request) {
  if (request.get_path().find("palette") != std::string::npos) {
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(JSON_palette_names, 200,
                                        "application/json"));
  }
  else if (request.get_path().find("effects") != std::string::npos) {
    json modeNames = serializeModeNames();
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(modeNames.dump(), 200,
                                        "application/json"));
  }
  else if (request.get_path().find("fxdata") != std::string::npos) {
    json modeData = serializeModeData();
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(modeData.dump(), 200,
                                        "application/json"));
  }
  return std::shared_ptr<httpserver::http_response>(
      new httpserver::string_response("{\"error\":3}", 503,
                                      "application/json"));
}

} // namespace ravecylinder