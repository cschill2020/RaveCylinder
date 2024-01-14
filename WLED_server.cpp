#include "WLED.h"
#include <httpserver.hpp>

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
  std::cout << "Reading: "<< filename << std::endl;
  return std::shared_ptr<httpserver::file_response>(
      new httpserver::file_response(filename, 200, getContentType(filename)));
}

std::shared_ptr<httpserver::http_response>
json_response::render_GET(const httpserver::http_request &req) {
  std::cout<<"json get: "<<req.get_path()<<std::endl;
  return serveJson(req);
}

std::shared_ptr<httpserver::http_response>
json_response::render_POST(const httpserver::http_request &req) {
  std::cout<<"json post: "<<req.get_path()<<std::endl;
  std::cout<<"post req content: "<<req.get_content()<<std::endl;
  return postJson(req);
}

} // namespace ravecylinder