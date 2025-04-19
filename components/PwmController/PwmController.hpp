// file   : PwmController.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <driver/ledc.h>

namespace esp32pp {

class PwmController {
public:
    PwmController(gpio_num_t pin, uint32_t frequency, uint32_t dutyCycle);

    void setDutyCycle(uint32_t value);

private:
    void init();

    gpio_num_t _pin;
    uint32_t _frequency;
    uint8_t _dutyCycle;
    ledc_channel_config_t _ledcChannelConfig{};
};

} // namespace esp32pp
