// file   : WiFiStation.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <esp_wifi.h>
#include <asio.hpp>

#include <functional>
#include <memory>
#include <string>

namespace esp32pp {

class WiFiStation : public std::enable_shared_from_this<WiFiStation> {
public:
    using Handler = std::function<void()>;

    explicit WiFiStation(asio::io_context& ioContext);
    ~WiFiStation();

    std::string getSSID() const;

    void setConfig(const std::string& ssid, const std::string& password);
    void setOnConnect(Handler&& handler);
    void setOnReconnecting(Handler&& handler);
    void setOnStop(Handler&& handler);

    void start();
    void stop();

private:
    using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

    static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
    static void ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

    void handleWiFiEvent(int32_t eventId);
    void handleIpEvent(int32_t eventId);
    void scheduleReconnect();

    asio::io_context& _ioContext;
    WorkGuard _workGuard{_ioContext.get_executor()};
    asio::steady_timer _retryTimer;
    Handler _onConnect;
    Handler _onReconnecting;
    Handler _onStop;
    esp_netif_t* _netif{nullptr};
    esp_event_handler_instance_t _eventWiFi{nullptr};
    esp_event_handler_instance_t _eventIp{nullptr};
};

} // namespace esp32pp
