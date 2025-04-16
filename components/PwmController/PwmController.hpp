#ifndef ESP32PP_PWMCONTROLLER_HPP
#define ESP32PP_PWMCONTROLLER_HPP

#include <driver/ledc.h>

namespace esp32pp {

class PwmController {
public:
    PwmController(gpio_num_t pin, uint32_t frequency, uint32_t dutyCycle);

    void setDutyCycle(uint32_t value);

private:
    void init();

    // @formatter:off
    gpio_num_t                  _pin;
    uint32_t                    _frequency;
    uint8_t                     _dutyCycle;
    ledc_channel_config_t       _ledcChannelConfig {};
    // @formatter:on
};

} // namespace esp32pp

#endif // ESP32PP_PWMCONTROLLER_HPP
