// file   : WiFiManager.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "WiFiManager.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "WiFiManager";

WiFiManager::WiFiManager(asio::any_io_executor executor)
    : _executor(std::move(executor))
    , _workGuard(_executor)
    , _retryTimer(_executor)
{
    _netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiManager::wifiEventHandler, this, &_eventWiFi
    ));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiManager::ipEventHandler, this, &_eventIp
    ));
}

WiFiManager::~WiFiManager()
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _eventIp));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, _eventWiFi));
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(_netif);
}

std::string WiFiManager::getSSID() const
{
    wifi_config_t wifiConfig = {};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifiConfig));

    auto* ssid = reinterpret_cast<char*>(wifiConfig.sta.ssid);

    return {ssid, strnlen(ssid, sizeof(wifiConfig.sta.ssid))};
}

void WiFiManager::setConfig(const std::string& ssid, const std::string& password)
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

void WiFiManager::setOnConnect(Handler&& handler)
{
    _onConnect = std::move(handler);
}

void WiFiManager::setOnReconnecting(Handler&& handler)
{
    _onReconnecting = std::move(handler);
}

void WiFiManager::setOnStop(Handler&& handler)
{
    _onStop = std::move(handler);
}

void WiFiManager::start()
{
    ESP_LOGI(TAG, "Starting Wi-Fi station...");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiManager::stop()
{
    ESP_LOGI(TAG, "Stopping Wi-Fi station...");
    esp_wifi_stop();
}

void WiFiManager::wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto self = static_cast<WiFiManager*>(arg)->shared_from_this();
    asio::post(self->_executor, [=] { self->handleWiFiEvent(eventId); });
}

void WiFiManager::ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    const auto self = static_cast<WiFiManager*>(arg)->shared_from_this();
    asio::post(self->_executor, [=] { self->handleIpEvent(eventId); });
}

void WiFiManager::handleWiFiEvent(int32_t eventId)
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

void WiFiManager::handleIpEvent(int32_t eventId)
{
    if (eventId == IP_EVENT_STA_GOT_IP) {
        // auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        // ESP_LOGI(TAG, "Wi-Fi connected, got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if (_onConnect) {
            _onConnect();
        }
    }
}

void WiFiManager::scheduleReconnect()
{
    _retryTimer.expires_after(std::chrono::seconds(10));
    _retryTimer.async_wait(
        [self = shared_from_this()](const asio::error_code& ec) {
            if (!ec) {
                esp_wifi_connect();
            }
        }
    );
}

} // namespace esp32pp
