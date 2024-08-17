#include "HttpServer.hpp"

#include <esp_log.h>

constexpr auto TAG = "HttpServer";

HttpServer::HttpServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.stack_size = 32768;
  ESP_LOGI(TAG, "Starting HTTP Server on port: %d", config.server_port);
  ESP_ERROR_CHECK(httpd_start(&m_server, &config));
}

HttpServer::~HttpServer()
{
  httpd_stop(m_server);
}

void HttpServer::addUriHandler(const httpd_uri_t& httpdUri)
{
  ESP_LOGI(TAG, "addUriHandler: %s", httpdUri.uri);
  ESP_ERROR_CHECK(httpd_register_uri_handler(m_server, &httpdUri));
}
