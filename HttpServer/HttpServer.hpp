#ifndef ESP32PP_HTTPSERVER_HPP
#define ESP32PP_HTTPSERVER_HPP

#include <esp_http_server.h>

class HttpServer
{
public:
  explicit HttpServer();
  ~HttpServer();

  void addUriHandler(const httpd_uri_t& httpdUri);

private:
  httpd_handle_t m_server {nullptr};
};

#endif // ESP32PP_HTTPSERVER_HPP
