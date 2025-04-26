// file   : Button.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include "Task.hpp"

#include <driver/gpio.h>

#include <functional>

namespace esp32pp {

class Button {
public:
    using Handler = std::function<void()>;

    explicit Button(gpio_num_t gpio, std::string name);
    ~Button();

    void setPressedHandler(Handler handler);
    void setReleasedHandler(Handler handler);

private:
    static void gpio_interrupt_handler(void* arg);

    void handleInterrupt();
    void debounce();

    const gpio_num_t _gpio;
    const std::string _name;
    portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
    Task _task;
    volatile uint32_t _debounceTimeout{0};
    Handler _pressedHandler;
    Handler _releasedHandler;
};

} // namespace esp32pp
