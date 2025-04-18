// file   : TM1637.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <driver/gpio.h>

#include <vector>

namespace esp32pp {

class TM1637 {
public:
    TM1637(gpio_num_t clk, gpio_num_t dio);

    void init();
    void setBrightness(uint8_t brightness);
    void displaySegments(const std::vector<uint8_t>& segments);
    uint8_t readKeys();

private:
    enum class Command : uint8_t {
        AutoAddressing  = 0x40,
        SetBrightness   = 0x88,
        DisplaySegments = 0xC0
    };

    void startSignal();
    void stopSignal();
    void writeByte(uint8_t data);

    gpio_num_t _clkPin;
    gpio_num_t _dioPin;
};

} // namespace esp32pp
