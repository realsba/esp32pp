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

  const gpio_num_t m_gpio;
  const std::string m_name;
  portMUX_TYPE m_mux = portMUX_INITIALIZER_UNLOCKED;
  Task m_task;
  volatile uint32_t m_debounceTimeout {0};
  Handler m_onPressed;
  Handler m_onReleased;
};

} // namespace esp32pp

#endif // ESP32PP_BUTTON_HPP
