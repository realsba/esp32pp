#include "TM1637.hpp"

#include <freertos/FreeRTOS.h>

namespace esp32pp {

constexpr auto TAG = "TM1637";

TM1637::TM1637(gpio_num_t clk, gpio_num_t dio)
    : _clkPin(clk), _dioPin(dio)
{
}

void TM1637::init()
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << _clkPin) | (1ULL << _dioPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
#if SOC_GPIO_SUPPORT_PIN_HYS_FILTER
        .hys_ctrl_mode = GPIO_HYS_SOFT_ENABLE
#endif
    };
    gpio_config(&config);

    startSignal();
    writeByte(static_cast<uint8_t>(Command::AutoAddressing));
    stopSignal();
}

void TM1637::setBrightness(uint8_t brightness)
{
    if (brightness > 7) {
        brightness = 7;
    }
    startSignal();
    writeByte(static_cast<uint8_t>(Command::SetBrightness) | brightness);
    stopSignal();
}

void TM1637::displaySegments(const std::vector<uint8_t>& segments)
{
    startSignal();
    writeByte(static_cast<uint8_t>(Command::DisplaySegments));
    for (auto segment : segments) {
        writeByte(segment);
    }
    stopSignal();
}

void TM1637::startSignal()
{
    gpio_set_level(_clkPin, 1);
    gpio_set_level(_dioPin, 1);
    esp_rom_delay_us(2);
    gpio_set_level(_dioPin, 0);
}

void TM1637::stopSignal()
{
    gpio_set_level(_clkPin, 0);
    gpio_set_level(_dioPin, 0);
    esp_rom_delay_us(2);
    gpio_set_level(_clkPin, 1);
    esp_rom_delay_us(2);
    gpio_set_level(_dioPin, 1);
}

void TM1637::writeByte(uint8_t data)
{
    for (int i = 0; i < 8; ++i) {
        gpio_set_level(_clkPin, 0);
        gpio_set_level(_dioPin, data & 0x01);
        esp_rom_delay_us(3);
        data >>= 1;
        gpio_set_level(_clkPin, 1);
        esp_rom_delay_us(3);
    }

    // ACK
    gpio_set_level(_clkPin, 0);
    gpio_set_direction(_dioPin, GPIO_MODE_INPUT);
    esp_rom_delay_us(5);
    gpio_set_level(_clkPin, 1);
    esp_rom_delay_us(2);
    gpio_set_direction(_dioPin, GPIO_MODE_OUTPUT);
}

} // namespace esp32pp
