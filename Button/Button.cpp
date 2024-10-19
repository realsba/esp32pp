#include "Button.hpp"

#include "CriticalSection.hpp"

#include <esp_log.h>

namespace esp32pp {

using namespace std;

Button::Button(gpio_num_t gpio, std::string name)
    : _gpio(gpio)
    , _name(std::move(name))
    , _task(std::bind_front(&Button::debounce, this), _name)
{
    gpio_config_t io_conf {};
    io_conf.intr_type    = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << _gpio);
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(_gpio, gpio_interrupt_handler, this);

    _task.suspend();
}

Button::~Button()
{
    _task.terminate();
}

void Button::setPressedHandler(Button::Handler&& handler)
{
    _onPressed = std::move(handler);
}

void Button::setReleasedHandler(Button::Handler&& handler)
{
    _onReleased = std::move(handler);
}

void IRAM_ATTR Button::gpio_interrupt_handler(void* arg)
{
    static_cast<Button*>(arg)->handleInterrupt();
}

void IRAM_ATTR Button::handleInterrupt()
{
    CriticalSectionIsr cs(_mux);
    _debounceTimeout = xTaskGetTickCountFromISR() + pdMS_TO_TICKS(10);
    _task.resumeFromISR();
}

void Button::debounce()
{
    while (true) {
        uint32_t debounceTimeout;

        {
            CriticalSection cs(_mux);
            debounceTimeout = _debounceTimeout;
        }

        if (xTaskGetTickCount() >= debounceTimeout) {
            if (gpio_get_level(_gpio)) {
                if (_onReleased) {
                    _onReleased();
                }
            } else {
                if (_onPressed) {
                    _onPressed();
                }
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    _task.suspend();
}

} // namespace esp32pp
