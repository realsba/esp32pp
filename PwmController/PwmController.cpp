#include "PwmController.hpp"

namespace esp32pp {

PwmController::PwmController(gpio_num_t pin, uint32_t frequency, uint32_t dutyCycle)
  : m_pin(pin), m_frequency(frequency), m_dutyCycle(dutyCycle)
{
  init();
}

void PwmController::setDutyCycle(uint32_t value)
{
  m_dutyCycle = value;
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, m_ledcChannel.channel, m_dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, m_ledcChannel.channel);
}

void PwmController::init()
{
  const auto timerConfig = ledc_timer_config_t {
    .speed_mode       = LEDC_HIGH_SPEED_MODE,
    .duty_resolution  = LEDC_TIMER_10_BIT,
    .timer_num        = LEDC_TIMER_0,
    .freq_hz          = m_frequency,
    .clk_cfg          = LEDC_AUTO_CLK,
    .deconfigure      = false
  };
  ledc_timer_config(&timerConfig);

  m_ledcChannel = {
    .gpio_num       = m_pin,
    .speed_mode     = LEDC_HIGH_SPEED_MODE,
    .channel        = LEDC_CHANNEL_0,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = LEDC_TIMER_0,
    .duty           = m_dutyCycle,
    .hpoint         = 0,
    .flags          = {.output_invert = false}
  };
  ledc_channel_config(&m_ledcChannel);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, m_ledcChannel.channel, m_dutyCycle);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, m_ledcChannel.channel);
}

} // namespace esp32pp
