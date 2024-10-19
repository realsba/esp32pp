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

    void setSSID(const std::string& ssid);
    void setPassword(const std::string& password);

    void start();
    void stop();

private:
    using WorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;

    static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
    static void ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

    void handleWiFiEvent(int32_t eventId);
    void handleIpEvent(int32_t eventId);
    void scheduleReconnect();

    asio::io_context& m_ioContext;
    WorkGuard m_workGuard {m_ioContext.get_executor()};
    asio::steady_timer m_retryTimer;
    Handler m_onConnect;
    Handler m_onStop;
    std::string m_ssid;
    std::string m_password;
    esp_netif_t* m_netif {nullptr};
    esp_event_handler_instance_t m_eventWiFi {nullptr};
    esp_event_handler_instance_t m_eventIp {nullptr};
};

} // namespace esp32pp

#endif  // ESP32PP_WIFI_STATION_HPP
