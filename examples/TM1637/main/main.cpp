#include <esp_log.h>

#include "TM1637.hpp"

#include "freertos/FreeRTOS.h"

constexpr auto TAG = "TM1637";

extern "C" void app_main()
{
    esp32pp::TM1637 display(GPIO_NUM_21, GPIO_NUM_22); // CLK = GPIO21, DIO = GPIO22
    display.init();

    const uint8_t digits[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    display.setBrightness(0x03);

    const uint8_t SEG_TOP = 0x01;
    const uint8_t SEG_TOP_RIGHT = 0x02;
    const uint8_t SEG_BOTTOM_RIGHT = 0x04;
    const uint8_t SEG_BOTTOM = 0x08;
    const uint8_t SEG_BOTTOM_LEFT = 0x10;
    const uint8_t SEG_TOP_LEFT = 0x20;

    const struct {
        int digit;
        uint8_t segment;
    } animations[12] = {
        {0, SEG_TOP},
        {0, SEG_TOP_LEFT},
        {0, SEG_BOTTOM_LEFT},
        {0, SEG_BOTTOM},
        {1, SEG_BOTTOM},
        {2, SEG_BOTTOM},
        {3, SEG_BOTTOM},
        {3, SEG_BOTTOM_RIGHT},
        {3, SEG_TOP_RIGHT},
        {3, SEG_TOP},
        {2, SEG_TOP},
        {1, SEG_TOP},
    };

    std::vector<uint8_t> segments = {0x00, 0x00, 0x00, 0x00};

    for (auto animation : animations) {
        int digit = animation.digit;
        uint8_t segment = animation.segment;
        segments[digit] |= segment;
        display.displaySegments(segments);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    int number = 0;
    vTaskDelay(pdMS_TO_TICKS(1));

    while (true) {
        int d[4] = {
            (number / 1000) % 10,
            (number / 100) % 10,
            (number / 10) % 10,
            number % 10
        };
        ++number;
        display.displaySegments({digits[d[0]], digits[d[1]], digits[d[2]], digits[d[3]]});
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
