#include "TelegramBot.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "TelegramBot";

TelegramBot::TelegramBot(const std::string& token)
{
  std::string url = "https://api.telegram.org/bot" + token + "/sendMessage";
  esp_http_client_config_t config {};
  config.url = url.c_str();
  m_httpClient = esp_http_client_init(&config);
  if (m_httpClient == nullptr) {
    ESP_LOGE(TAG, "Failed to create HTTP client");
    return;
  }

  ESP_ERROR_CHECK(esp_http_client_set_method(m_httpClient, HTTP_METHOD_POST));
  ESP_ERROR_CHECK(esp_http_client_set_header(m_httpClient, "Content-Type", "application/json"));
}

TelegramBot::~TelegramBot()
{
  ESP_ERROR_CHECK(esp_http_client_cleanup(m_httpClient));
}

void TelegramBot::sendMessage(const std::string& channelId, const std::string& message)
{
  std::string payload = R"({"chat_id":")" + channelId + R"(","text":")" + message + "\"}";
  ESP_ERROR_CHECK(esp_http_client_set_post_field(m_httpClient, payload.c_str(), payload.length()));

  auto err = esp_http_client_perform(m_httpClient);
  if (err == ESP_OK) {
    ESP_LOGI(
      TAG, "HTTP POST Status = %d, content_length = %lld",
      esp_http_client_get_status_code(m_httpClient),
      esp_http_client_get_content_length(m_httpClient)
    );
  } else {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
  }
}

} // namespace esp32pp
