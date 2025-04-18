// file   : HttpServer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <esp_http_server.h>

namespace esp32pp {

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    void addUriHandler(const httpd_uri_t& httpdUri);

private:
    httpd_handle_t _server{nullptr};
};

} // namespace esp32pp
