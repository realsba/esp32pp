// file   : WiFiManager.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <esp_wifi.h>
#include <asio.hpp>

#include <functional>
#include <memory>
#include <string>

namespace esp32pp {

class WiFiManager : public std::enable_shared_from_this<WiFiManager> {
public:
    using Handler = std::function<void()>;

    explicit WiFiManager(asio::any_io_executor executor);
    ~WiFiManager();

    std::string getSSID() const;

    void setConfig(const std::string& ssid, const std::string& password);
    void setOnConnect(Handler&& handler);
    void setOnReconnecting(Handler&& handler);
    void setOnStop(Handler&& handler);

    void start();
    void stop();

private:
    using WorkGuard = asio::executor_work_guard<asio::any_io_executor>;

    static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
    static void ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

    void handleWiFiEvent(int32_t eventId);
    void handleIpEvent(int32_t eventId);
    void scheduleReconnect();

    asio::any_io_executor _executor;
    WorkGuard _workGuard;
    asio::steady_timer _retryTimer;
    Handler _onConnect;
    Handler _onReconnecting;
    Handler _onStop;
    esp_netif_t* _netif{nullptr};
    esp_event_handler_instance_t _eventWiFi{nullptr};
    esp_event_handler_instance_t _eventIp{nullptr};
};

} // namespace esp32pp
