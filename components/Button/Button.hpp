#ifndef ESP32PP_BUTTON_HPP
#define ESP32PP_BUTTON_HPP

#include "Task.hpp"

#include <driver/gpio.h>

#include <functional>

namespace esp32pp {

class Button {
public:
    using Handler = std::function<void()>;

    explicit Button(gpio_num_t gpio, std::string name);
    ~Button();

    void setPressedHandler(Handler&& handler);
    void setReleasedHandler(Handler&& handler);

private:
    static void gpio_interrupt_handler(void* arg);

    void handleInterrupt();
    void debounce();

    // @formatter:off
    const gpio_num_t            _gpio;
    const std::string           _name;
    portMUX_TYPE                _mux = portMUX_INITIALIZER_UNLOCKED;
    Task                        _task;
    volatile uint32_t           _debounceTimeout {0};
    Handler                     _onPressed;
    Handler                     _onReleased;
    // @formatter:on
};

} // namespace esp32pp

#endif // ESP32PP_BUTTON_HPP
