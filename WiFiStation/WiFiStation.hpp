#ifndef ESP32PP_WIFI_STATION_HPP
#define ESP32PP_WIFI_STATION_HPP

#include <esp_wifi.h>

#include <freertos/event_groups.h>

#include <system_error>
#include <functional>
#include <string>

class WiFiStation
{
public:
  using Handler = std::function<void()>;

  explicit WiFiStation(Handler&& onConnect, Handler&& onStop);
  ~WiFiStation();

  void setSSID(const std::string& ssid);
  void setPassword(const std::string& password);

  void start();
  void stop();

private:
  static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
  static void ipEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
  void handleWiFiEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData);
  void handleIpEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData);

private:
  Handler m_onConnect;
  Handler m_onStop;
  std::string m_ssid;
  std::string m_password;
  esp_netif_t* m_netif {nullptr};
  EventGroupHandle_t m_wifiEventGroup {nullptr};
  esp_event_handler_instance_t m_eventWiFi {nullptr};
  esp_event_handler_instance_t m_eventIp {nullptr};
};

#endif // ESP32PP_WIFI_STATION_HPP
