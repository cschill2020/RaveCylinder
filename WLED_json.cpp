#include "WLED.h"
#include "WLED_FX.h"
#include <httpserver.hpp>

namespace ravecylinder {
std::shared_ptr<httpserver::http_response>
serveJson(const httpserver::http_request& request) {
  if (request.get_path().find("palette") != std::string::npos) {
    return std::shared_ptr<httpserver::http_response>(
        new httpserver::string_response(JSON_palette_names, 200,
                                        "application/json"));
  }
  return std::shared_ptr<httpserver::http_response>(
      new httpserver::string_response("{\"error\":3}", 503,
                                      "application/json"));
}

} // namespace ravecylinder