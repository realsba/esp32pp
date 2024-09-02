#ifndef ESP32PP_HTTPSERVER_HPP
#define ESP32PP_HTTPSERVER_HPP

#include <esp_http_server.h>

namespace esp32pp {

class HttpServer {
public:
  explicit HttpServer();
  ~HttpServer();

  void addUriHandler(const httpd_uri_t& httpdUri);

private:
  httpd_handle_t m_server {nullptr};
};

} // namespace esp32pp

#endif // ESP32PP_HTTPSERVER_HPP
