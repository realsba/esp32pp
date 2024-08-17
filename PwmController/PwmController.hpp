#ifndef ESP32PP_PWMCONTROLLER_HPP
#define ESP32PP_PWMCONTROLLER_HPP

#include <driver/ledc.h>

class PwmController {
public:
  PwmController(gpio_num_t pin, uint32_t frequency, uint32_t dutyCycle);

  void setDutyCycle(uint32_t value);

private:
  void init();

  gpio_num_t                    m_pin;
  uint32_t                      m_frequency;
  uint8_t                       m_dutyCycle;
  ledc_channel_config_t         m_ledcChannel {};
};

#endif // ESP32PP_PWMCONTROLLER_HPP
