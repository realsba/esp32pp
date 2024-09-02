#include "Timer.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "Timer";

Timer::Timer(Callback callback)
  : m_callback(std::move(callback))
{
  esp_timer_create_args_t timerArgs = {
    .callback = &Timer::timerCallback,
    .arg = this,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "Timer",
    .skip_unhandled_events = true
  };

  auto err = esp_timer_create(&timerArgs, &m_timerHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create timer: %s", esp_err_to_name(err));
  }
}

Timer::~Timer()
{
  stop();
  if (m_timerHandle) {
    esp_timer_delete(m_timerHandle);
  }
}

void Timer::startPeriodic(Duration interval)
{
  if (m_timerHandle && !m_isRunning) {
    ESP_ERROR_CHECK(esp_timer_start_periodic(m_timerHandle, interval.count()));
    m_isRunning = true;
  }
}

void Timer::startOnce(Duration timeout)
{
  if (m_timerHandle && !m_isRunning) {
    ESP_ERROR_CHECK(esp_timer_start_once(m_timerHandle, timeout.count()));
    m_isRunning = true;
  }
}

void Timer::stop()
{
  if (m_timerHandle && m_isRunning) {
    esp_timer_stop(m_timerHandle);
    m_isRunning = false;
  }
}

bool Timer::isRunning() const
{
  return m_isRunning;
}

void Timer::timerCallback(void* arg)
{
  auto* timer = static_cast<Timer*>(arg);
  if (timer && timer->m_callback) {
    timer->m_callback();
  }
}

} // esp32pp
