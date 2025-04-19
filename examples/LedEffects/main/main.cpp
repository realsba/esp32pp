#include "LedController.hpp"
#include "ActionSequence.hpp"

#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_log.h>

constexpr auto TAG = "LedEffect Example";

using namespace std::chrono_literals;

extern "C" void app_main()
{
    auto err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    asio::io_context ioc;
    esp32pp::LedController led(2, LEDC_CHANNEL_0);

    esp32pp::ActionSequence fastBlink(ioc.get_executor());
    esp32pp::ActionSequence actions(ioc.get_executor());

    fastBlink.addAction([&led] { led.turnOn(); }, 50ms);
    fastBlink.addAction([&led] { led.turnOff(); }, 50ms);
    fastBlink.setRepeatCount(3);

    actions.addAction([&fastBlink] { fastBlink.setRepeatCount(3); fastBlink.start(); }, 1s);
    actions.addAction([&led] { led.fade(1, 500ms); }, 900ms);
    actions.addAction([&led] { led.turnOff(); }, 100ms);

    actions.start();

    ioc.run();
}
