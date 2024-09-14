#include "WiFiStation.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "WiFiStation";

constexpr uint32_t WIFI_CONNECTED_BIT = BIT0;
constexpr uint32_t WIFI_FAIL_BIT      = BIT1;

WiFiStation::WiFiStation(Handler&& onConnect, Handler&& onStop)
    : m_onConnect(std::move(onConnect))
    , m_onStop(std::move(onStop))
{
    m_netif                = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiStation::wifiEventHandler, this, &m_eventWiFi
    ));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiStation::ipEventHandler, this, & m_eventIp
    ));
}

WiFiStation::~WiFiStation()
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, m_eventIp));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, m_eventWiFi));
    vEventGroupDelete(m_wifiEventGroup);
    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(m_netif);
}

void WiFiStation::setSSID(const std::string& ssid)
{
    m_ssid = ssid;
}

void WiFiStation::setPassword(const std::string& password)
{
    m_password = password;
}

void WiFiStation::start()
{
    ESP_LOGI(TAG, "Starting WiFi station");

    m_wifiEventGroup = xEventGroupCreate();

    wifi_config_t wifiConfig = {};
    auto n = m_ssid.copy(reinterpret_cast<char*>(wifiConfig.sta.ssid), sizeof(wifiConfig.sta.ssid) - 1);
    wifiConfig.sta.ssid[n] = 0;
    n = m_password.copy(reinterpret_cast<char*>(wifiConfig.sta.password), sizeof(wifiConfig.sta.password) - 1);
    wifiConfig.sta.password[n] = 0;
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));
    ESP_ERROR_CHECK(esp_wifi_start());

    auto bits = xEventGroupWaitBits(
        m_wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP SSID: %s password: ****************", m_ssid.c_str());
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", m_ssid.c_str());
    } else {
        ESP_LOGE(TAG, "Unexpected event");
    }
}

void WiFiStation::stop()
{
    esp_wifi_stop();
}

void WiFiStation::wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    static_cast<WiFiStation*>(arg)->handleWiFiEvent(eventBase, eventId, eventData);
}

void WiFiStation::ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    static_cast<WiFiStation*>(arg)->handleIpEvent(eventBase, eventId, eventData);
}

void WiFiStation::handleWiFiEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    if (eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (eventId == WIFI_EVENT_STA_STOP) {
        if (m_onStop) {
            m_onStop();
        }
    } else if (eventId == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGE(TAG, "Station disconnected");
        xEventGroupSetBits(m_wifiEventGroup, WIFI_FAIL_BIT);
    } else {
        ESP_LOGE(TAG, "Unexpected event: %ld", eventId);
    }
}

void WiFiStation::handleIpEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    if (eventId == IP_EVENT_STA_GOT_IP) {
        auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        ESP_LOGI(TAG, "Station got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(m_wifiEventGroup, WIFI_CONNECTED_BIT);
        if (m_onConnect) {
            m_onConnect();
        }
    }
}

} // namespace esp32pp
