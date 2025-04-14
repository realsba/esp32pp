// file   : LedController.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "LedController.hpp"

#include <cmath>
#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "LedController";

LedController::LedController(int gpioNum, ledc_channel_t channel)
    : _channel(channel)
    , _maxDuty((1 << LEDC_TIMER_13_BIT) - 1)
{
    ledc_timer_config_t ledcTimerConfig = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledcTimerConfig));

    ledc_channel_config_t ledcChannelConfig = {
        .gpio_num = gpioNum,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {.output_invert = false}
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledcChannelConfig));

    ESP_ERROR_CHECK(ledc_fade_func_install(0));
}

void LedController::turnOn()
{
    if (!_isOn) {
        setDutyCycle(_brightness);
        _isOn = true;
    }
}

void LedController::turnOff()
{
    if (_isOn) {
        setDutyCycle(0.0f);
        _isOn = false;
    }
}

bool LedController::isOn() const
{
    return _isOn;
}

void LedController::setBrightness(float brightness)
{
    if (brightness < 0.0f || brightness > 1.0f) {
        throw std::out_of_range("Brightness must be in range [0.0, 1.0]");
    }
    _brightness = brightness;
    if (_isOn) {
        setDutyCycle(_brightness);
    }
}

float LedController::getBrightness() const
{
    return _brightness;
}

void LedController::fade(float brightness, std::chrono::milliseconds duration)
{
    if (brightness < 0.0f || brightness > 1.0f) {
        throw std::out_of_range("Target brightness must be in range [0.0, 1.0]");
    }

    if (duration <= std::chrono::milliseconds(0)) {
        throw std::invalid_argument("Duration must be positive");
    }

    auto duty = static_cast<uint32_t>(std::round(brightness * _maxDuty));
    ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, _channel, duty, duration.count()));
    ESP_ERROR_CHECK(ledc_fade_start(LEDC_HIGH_SPEED_MODE, _channel, LEDC_FADE_NO_WAIT));

    _brightness = brightness;
}

void LedController::setDutyCycle(float brightness)
{
    auto duty = static_cast<uint32_t>(std::round(brightness * _maxDuty));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, _channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, _channel));
}

} // namespace esp32pp
