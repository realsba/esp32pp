// file   : PwmController.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "PwmController.hpp"

namespace esp32pp {

PwmController::PwmController(gpio_num_t pin, uint32_t frequency, uint32_t dutyCycle)
    : _pin(pin)
    , _frequency(frequency)
    , _dutyCycle(dutyCycle)
{
    init();
}

void PwmController::setDutyCycle(uint32_t value)
{
    _dutyCycle = value;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, _ledcChannelConfig.channel, _dutyCycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, _ledcChannelConfig.channel);
}

void PwmController::init()
{
    const auto timerConfig = ledc_timer_config_t{
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = _frequency,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ledc_timer_config(&timerConfig);

    _ledcChannelConfig = {
        .gpio_num = _pin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = _dutyCycle,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {.output_invert = false}
    };
    ledc_channel_config(&_ledcChannelConfig);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, _ledcChannelConfig.channel, _dutyCycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, _ledcChannelConfig.channel);
}

} // namespace esp32pp
