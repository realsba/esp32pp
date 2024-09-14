#include "Button.hpp"

#include "CriticalSection.hpp"

#include <esp_log.h>

namespace esp32pp {

using namespace std;

Button::Button(gpio_num_t gpio, std::string name)
    : m_gpio(gpio)
    , m_name(std::move(name))
    , m_task(std::bind_front(&Button::debounce, this), m_name)
{
    gpio_config_t io_conf {};
    io_conf.intr_type    = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << m_gpio);
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(m_gpio, gpio_interrupt_handler, this);

    m_task.suspend();
}

Button::~Button()
{
    m_task.terminate();
}

void Button::setPressedHandler(Button::Handler&& handler)
{
    m_onPressed = std::move(handler);
}

void Button::setReleasedHandler(Button::Handler&& handler)
{
    m_onReleased = std::move(handler);
}

void IRAM_ATTR Button::gpio_interrupt_handler(void* arg)
{
    static_cast<Button*>(arg)->handleInterrupt();
}

void IRAM_ATTR Button::handleInterrupt()
{
    CriticalSectionIsr cs(m_mux);
    m_debounceTimeout = xTaskGetTickCountFromISR() + pdMS_TO_TICKS(10);
    m_task.resumeFromISR();
}

void Button::debounce()
{
    while (true) {
        uint32_t debounceTimeout;

        {
            CriticalSection cs(m_mux);
            debounceTimeout = m_debounceTimeout;
        }

        if (xTaskGetTickCount() >= debounceTimeout) {
            if (gpio_get_level(m_gpio)) {
                if (m_onReleased) {
                    m_onReleased();
                }
            } else {
                if (m_onPressed) {
                    m_onPressed();
                }
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    m_task.suspend();
}

} // namespace esp32pp
