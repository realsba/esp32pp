#include "DHT.hpp"

#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>

#include <memory>

static const char* TAG = "main";

extern "C" void app_main(void)
{
  auto ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  auto sensor = std::make_unique<DHT22>();
  sensor->setup(static_cast<gpio_num_t>(CONFIG_RMT_GPIO_NUM), 1000000);

  auto onReadSensor = [&](std::error_code ec, float humidity, float temperature)
  {
    ESP_LOGI(TAG, "onReadSensor. humidity: %f, temperature: %f", humidity, temperature);
  };

  vTaskDelay(pdMS_TO_TICKS(1000));

  while (true) {
    sensor->read(onReadSensor);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
