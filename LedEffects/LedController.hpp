#ifndef ESP32PP_LEDCONTROLLER_HPP
#define ESP32PP_LEDCONTROLLER_HPP

#include <driver/ledc.h>

#include <chrono>

namespace esp32pp {

class LedController {
public:
    LedController(int gpioNum, ledc_channel_t channel);

    void turnOn();
    void turnOff();
    [[nodiscard]] bool isOn() const;

    void setBrightness(float brightness);
    [[nodiscard]] float getBrightness() const;

    void fade(float brightness, std::chrono::milliseconds duration);

private:
    void setDutyCycle(float brightness);

    ledc_channel_t _channel;
    const uint32_t _maxDuty;
    float _brightness {1.0f};
    bool _isOn {false};
};


} // namespace esp32pp

#endif // ESP32PP_LEDCONTROLLER_HPP
