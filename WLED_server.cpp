#include "WLED.h"
#include <httpserver.hpp>
#include <string>

namespace ravecylinder {

static std::string getContentType(std::string filename) {
  if (filename.find(".htm") != std::string::npos)
    return "text/html";
  else if (filename.find(".html") != std::string::npos)
    return "text/html";
  else if (filename.find(".css") != std::string::npos)
    return "text/css";
  else if (filename.find(".js") != std::string::npos)
    return "application/javascript";
  else if (filename.find(".json") != std::string::npos)
    return "application/json";
  else if (filename.find(".png") != std::string::npos)
    return "image/png";
  else if (filename.find(".gif") != std::string::npos)
    return "image/gif";
  else if (filename.find(".jpg") != std::string::npos)
    return "image/jpeg";
  else if (filename.find(".ico") != std::string::npos)
    return "image/x-icon";
  //  else if(filename.endsWith(".xml")) return "text/xml";
  //  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  //  else if(filename.endsWith(".zip")) return "application/x-zip";
  //  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

std::shared_ptr<httpserver::http_response>
index_response::render_GET(const httpserver::http_request &req) {
  std::string base_filename = "../data";
  std::string filename;
  std::string_view path = req.get_path();
  if (req.get_path() == "/") {
    filename = base_filename + "/simple.htm";
  } else {
    filename = base_filename + std::string(req.get_path());
  }
  std::cout << "Reading: " << filename << std::endl;
  return std::shared_ptr<httpserver::file_response>(
      new httpserver::file_response(filename, 200, getContentType(filename)));
}

std::shared_ptr<httpserver::http_response>
json_response::render_GET(const httpserver::http_request &req) {
  //std::cout << "json get: " << req.get_path() << std::endl;
  return serveJson(req);
}

std::shared_ptr<httpserver::http_response>
json_response::render_POST(const httpserver::http_request &req) {
  //std::cout << "json post: " << req.get_path() << std::endl;
  //std::cout << "post req content: " << req.get_content() << std::endl;
  std::cout << "Json Post Request: " << req.get_content() << std::endl;
  return postJson(req);
}

std::shared_ptr<httpserver::http_response>
serveSettingsJS(const httpserver::http_request &req) {
  char buf[SETTINGS_STACK_BUF_SIZE + 37];
  buf[0] = 0;
  int subPage = std::stoi(req.get_arg(F("p")).get_flat_value().data());
  if (subPage > 10) {
    strcpy_P(buf,
             PSTR("alert('Settings for this request are not implemented.');"));
    return std::shared_ptr<httpserver::string_response>(
        new httpserver::string_response(buf, 501, "application/javascript"));
  }
  //if (subPage > 0 /*&& !correctPIN && strlen(settingsPIN)>0*/) {
  //  strcpy_P(buf, PSTR("alert('PIN incorrect.');"));
  //  return std::shared_ptr<httpserver::string_response>(
  //      new httpserver::string_response(buf, 401, "application/javascript"));
 // }
  strcat(buf, "function GetV(){var d=document;");
  getSettingsJS(subPage, buf + strlen(buf)); // this may overflow by 35bytes!!!
  strcat(buf, "}");
  httpserver::string_response *response =
      new httpserver::string_response(buf, 200, "application/javascript");
  response->with_header(F("Cache-Control"), "no-store");
  response->with_header(F("Expires"), "0");
  return std::shared_ptr<httpserver::string_response>(response);
}

std::shared_ptr<httpserver::http_response>
serveSettings(const httpserver::http_request &req, bool post = false) {
  std::cout<<"Serving Settings"<<std::endl;
  std::string base_filename = "../data";
  std::string filename;
  std::string_view path = req.get_path();
  std::cout << "getpath="<<req.get_path() << std::endl;
  int subPage = 0, originalSubPage = 0;

  if (req.get_path().find("sett") != std::string::npos) {
    if (req.get_path().find(".js") != std::string::npos)
      subPage = SUBPAGE_JS;
    else if (req.get_path().find(".css") != std::string::npos)
      subPage = SUBPAGE_CSS;
    else if (req.get_path().find("leds") != std::string::npos) subPage = SUBPAGE_LEDS;
    else if (req.get_path().find("2D") != std::string::npos) subPage = SUBPAGE_2D;

  }
  if (post) { //settings/set POST request, saving
    //if (subPage != SUBPAGE_WIFI || !(wifiLock && otaLock)) 
    std::cout<<"Posting settings!"<<std::endl;
    handleSettingsSet(req, subPage);
  }

  switch (subPage) {
  case SUBPAGE_JS:
    std::cout<<"servesettingsjs"<<std::endl;
    return serveSettingsJS(req);
  case SUBPAGE_CSS:
    filename = base_filename + "/style.css";
    break;
  case SUBPAGE_LEDS:
    filename = base_filename + "/settings_leds.htm";
    break;
  case SUBPAGE_2D:
    filename = base_filename + "/settings_2D.htm";
    break;
  default:
    filename = base_filename + "/settings.htm";
  }
  std::cout<<"filename = "<<filename<<std::endl;
  return std::shared_ptr<httpserver::file_response>(
      new httpserver::file_response(filename, 200, getContentType(filename)));
}

std::shared_ptr<httpserver::http_response>
settings_response::render_GET(const httpserver::http_request &req) {
  std::cout << "settings get: " << req.get_path() << std::endl;
  return serveSettings(req);
}

std::shared_ptr<httpserver::http_response>
settings_response::render_POST(const httpserver::http_request &req) {
std::cout<<"settings post: "<<req.get_path()<<std::endl;
return serveSettings(req, true);
}

} // namespace ravecylinder