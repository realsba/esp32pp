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
    explicit WiFiManager(asio::any_io_executor executor);

public:
    using Handler = std::function<void()>;

    enum class WifiMode : uint8_t {
        Station = 0,
        AccessPoint = 1,
        ApSta = 2,
        Unknown = 255
    };

    ~WiFiManager();

    static std::shared_ptr<WiFiManager> create(asio::any_io_executor ex);

    void setStationConfig(const std::string& ssid, const std::string& password);
    void setAccessPointConfig(const std::string& ssid, const std::string& password);
    std::string getStationSSID() const;

    WifiMode getWifiMode() const;
    void setWifiMode(WifiMode mode);

    void setConnectHandler(Handler handler);
    void setReconnectingHandler(Handler handler);
    void setStopHandler(Handler handler);

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
    Handler _connectHandler;
    Handler _reconnectingHandler;
    Handler _stopHandler;
    esp_netif_t* _netifSta{nullptr};
    esp_netif_t* _netifAp{nullptr};
    esp_event_handler_instance_t _eventWiFi{nullptr};
    esp_event_handler_instance_t _eventIp{nullptr};
};

inline const char* toString(WiFiManager::WifiMode mode) {
    switch (mode) {
        case WiFiManager::WifiMode::Station:     return "Station";
        case WiFiManager::WifiMode::AccessPoint: return "AccessPoint";
        case WiFiManager::WifiMode::ApSta:       return "Station+AP";
        default:                                 return "Unknown";
    }
}

} // namespace esp32pp
