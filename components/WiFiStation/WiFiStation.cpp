#include "WiFiStation.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "WiFiStation";

WiFiStation::WiFiStation(asio::io_context& ioContext)
    : _ioContext(ioContext)
    , _retryTimer(ioContext)
{
    _netif                 = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiStation::wifiEventHandler, this, &_eventWiFi
    ));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiStation::ipEventHandler, this, &_eventIp
    ));
}

WiFiStation::~WiFiStation()
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _eventIp));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, _eventWiFi));
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(_netif);
}

std::string WiFiStation::getSSID() const
{
    wifi_config_t wifiConfig = {};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifiConfig));

    auto* ssid = reinterpret_cast<char*>(wifiConfig.sta.ssid);

    return {ssid, strnlen(ssid, sizeof(wifiConfig.sta.ssid))};
}

void WiFiStation::setConfig(const std::string& ssid, const std::string& password)
{
    wifi_config_t wifiConfig = {};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifiConfig));

    auto n = ssid.copy(reinterpret_cast<char*>(wifiConfig.sta.ssid), sizeof(wifiConfig.sta.ssid) - 1);
    wifiConfig.sta.ssid[n] = 0;
    n = password.copy(reinterpret_cast<char*>(wifiConfig.sta.password), sizeof(wifiConfig.sta.password) - 1);
    wifiConfig.sta.password[n] = 0;
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
}

void WiFiStation::setOnConnect(Handler&& handler)
{
    _onConnect = std::move(handler);
}

void WiFiStation::setOnReconnecting(Handler&& handler)
{
    _onReconnecting = std::move(handler);
}

void WiFiStation::setOnStop(Handler&& handler)
{
    _onStop = std::move(handler);
}

void WiFiStation::start()
{
    ESP_LOGI(TAG, "Starting Wi-Fi station...");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiStation::stop()
{
    ESP_LOGI(TAG, "Stopping Wi-Fi station...");
    esp_wifi_stop();
}

void WiFiStation::wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto self = static_cast<WiFiStation*>(arg)->shared_from_this();
    asio::post(self->_ioContext, [=] { self->handleWiFiEvent(eventId); });
}

void WiFiStation::ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    const auto self = static_cast<WiFiStation*>(arg)->shared_from_this();
    asio::post(self->_ioContext, [=] { self->handleIpEvent(eventId); });
}

void WiFiStation::handleWiFiEvent(int32_t eventId)
{
    if (eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (eventId == WIFI_EVENT_STA_STOP) {
        if (_onStop) {
            _onStop();
        }
    } else if (eventId == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi Disconnected. Scheduling reconnect...");
        if (_onReconnecting) {
            _onReconnecting();
        }
        scheduleReconnect();
    }
}

void WiFiStation::handleIpEvent(int32_t eventId)
{
    if (eventId == IP_EVENT_STA_GOT_IP) {
        // auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        // ESP_LOGI(TAG, "Wi-Fi connected, got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if (_onConnect) {
            _onConnect();
        }
    }
}

void WiFiStation::scheduleReconnect()
{
    _retryTimer.expires_after(std::chrono::seconds(10));
    _retryTimer.async_wait(
        [self = shared_from_this()](const asio::error_code& ec)
        {
            if (!ec) {
                esp_wifi_connect();
            }
        }
    );
}

} // namespace esp32pp
