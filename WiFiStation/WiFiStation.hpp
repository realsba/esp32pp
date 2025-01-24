#ifndef ESP32PP_WIFI_STATION_HPP
#define ESP32PP_WIFI_STATION_HPP

#include <esp_wifi.h>
#include <asio.hpp>

#include <functional>
#include <memory>
#include <string>

namespace esp32pp {

class WiFiStation : public std::enable_shared_from_this<WiFiStation> {
public:
    using Handler = std::function<void()>;

    explicit WiFiStation(asio::io_context& ioContext, Handler&& onConnect, Handler&& onStop);
    ~WiFiStation();

    std::string getSSID() const;

    void setConfig(const std::string& ssid, const std::string& password);

    void start();
    void stop();

private:
    using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

    static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
    static void ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

    void handleWiFiEvent(int32_t eventId);
    void handleIpEvent(int32_t eventId);
    void scheduleReconnect();
    void updateConfig();

    asio::io_context& _ioContext;
    WorkGuard _workGuard {_ioContext.get_executor()};
    asio::steady_timer _retryTimer;
    Handler _onConnect;
    Handler _onStop;
    esp_netif_t* _netif {nullptr};
    esp_event_handler_instance_t _eventWiFi {nullptr};
    esp_event_handler_instance_t _eventIp {nullptr};
};

} // namespace esp32pp

#endif  // ESP32PP_WIFI_STATION_HPP
