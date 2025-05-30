// file   : TelegramBot.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <esp_http_client.h>

#include <string>

namespace esp32pp {

class TelegramBot {
public:
    explicit TelegramBot(const std::string& token);
    ~TelegramBot();

    void sendMessage(const std::string& channelId, const std::string& message);

private:
    esp_http_client_handle_t _httpClient;
};

} // namespace esp32pp
