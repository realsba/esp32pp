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
    if (_netifSta) {
        esp_netif_destroy_default_wifi(_netifSta);
    }
    if (_netifAp) {
        esp_netif_destroy_default_wifi(_netifAp);
    }
}

std::shared_ptr<WiFiManager> WiFiManager::create(asio::any_io_executor ex)
{
    return std::shared_ptr<WiFiManager>(new WiFiManager(std::move(ex)));
}

void WiFiManager::setStationConfig(const std::string& ssid, const std::string& password)
{
    wifi_config_t config = {};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &config));

    auto n = ssid.copy(reinterpret_cast<char*>(config.sta.ssid), sizeof(config.sta.ssid) - 1);
    config.sta.ssid[n] = 0;
    n = password.copy(reinterpret_cast<char*>(config.sta.password), sizeof(config.sta.password) - 1);
    config.sta.password[n] = 0;
    config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    config.sta.pmf_cfg.capable = true;
    config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
}

void WiFiManager::setAccessPointConfig(const std::string& ssid, const std::string& password)
{
    wifi_config_t config = {};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_AP, &config));

    auto n = ssid.copy(reinterpret_cast<char*>(config.ap.ssid), sizeof(config.ap.ssid) - 1);
    config.ap.ssid[n] = 0;
    n = password.copy(reinterpret_cast<char*>(config.ap.password), sizeof(config.ap.password) - 1);
    config.ap.password[n] = 0;
    config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    config.ap.ssid_len = 0;
    config.ap.max_connection = 4;
    config.ap.ssid_hidden = 0;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
}

std::string WiFiManager::getStationSSID() const
{
    wifi_config_t wifiConfig = {};
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifiConfig));
    auto* ssid = reinterpret_cast<char*>(wifiConfig.sta.ssid);
    return {ssid, strnlen(ssid, sizeof(wifiConfig.sta.ssid))};
}

WiFiManager::WifiMode WiFiManager::getWifiMode() const
{
    wifi_mode_t nativeMode;
    auto err = esp_wifi_get_mode(&nativeMode);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get Wi-Fi mode: %s (%d)", esp_err_to_name(err), err);
        return WifiMode::Unknown;
    }

    switch (nativeMode) {
        case WIFI_MODE_STA: return WifiMode::Station;
        case WIFI_MODE_AP: return WifiMode::AccessPoint;
        case WIFI_MODE_APSTA: return WifiMode::ApSta;
        default:
            return WifiMode::Unknown;
    }
}

void WiFiManager::setWifiMode(WifiMode mode)
{
    ESP_LOGI(TAG, "Switching Wi-Fi mode to %s", toString(mode));

    auto nativeMode = WIFI_MODE_NULL;
    switch (mode) {
        case WifiMode::Station:
            nativeMode = WIFI_MODE_STA;
            break;
        case WifiMode::AccessPoint:
            nativeMode = WIFI_MODE_AP;
            break;
        case WifiMode::ApSta:
            nativeMode = WIFI_MODE_APSTA;
            break;
        default:
            ESP_LOGE(TAG, "Unsupported WifiMode");
            return;
    }

    stop();
    ESP_ERROR_CHECK(esp_wifi_set_mode(nativeMode));
    start();
}

void WiFiManager::setConnectHandler(Handler handler)
{
    _connectHandler = std::move(handler);
}

void WiFiManager::setReconnectingHandler(Handler handler)
{
    _reconnectingHandler = std::move(handler);
}

void WiFiManager::setStopHandler(Handler handler)
{
    _stopHandler = std::move(handler);
}

void WiFiManager::start()
{
    wifi_mode_t currentMode;
    auto err = esp_wifi_get_mode(&currentMode);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current Wi-Fi mode");
        return;
    }

    if ((currentMode == WIFI_MODE_STA || currentMode == WIFI_MODE_APSTA) && !_netifSta) {
        _netifSta = esp_netif_create_default_wifi_sta();
    }
    if ((currentMode == WIFI_MODE_AP || currentMode == WIFI_MODE_APSTA) && !_netifAp) {
        _netifAp = esp_netif_create_default_wifi_ap();
    }

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiManager::stop()
{
    ESP_LOGI(TAG, "Stopping Wi-Fi...");
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
        if (_stopHandler) {
            _stopHandler();
        }
    } else if (eventId == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi Disconnected. Scheduling reconnect...");
        if (_reconnectingHandler) {
            _reconnectingHandler();
        }
        scheduleReconnect();
    }
}

void WiFiManager::handleIpEvent(int32_t eventId)
{
    if (eventId == IP_EVENT_STA_GOT_IP) {
        if (_connectHandler) {
            _connectHandler();
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
