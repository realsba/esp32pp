#include "Timer.hpp"

#include <esp_log.h>

constexpr auto TAG = "main";

using namespace std::chrono_literals;

using TimerUPtr = std::unique_ptr<esp32pp::Timer>;

TimerUPtr timer, timerOnce;

extern "C" void app_main()
{
  ESP_LOGI(TAG, "Timer example");

  timer = std::make_unique<esp32pp::Timer>(
    []
    {
      ESP_LOGI(TAG, "Periodic timer callback triggered");
    }
  );

  timerOnce = std::make_unique<esp32pp::Timer>(
    []
    {
      ESP_LOGI(TAG, "One-shot timer callback triggered, stopping periodic timer");
      timerOnce->stop();
      timerOnce.reset();
    }
  );

  timer->startPeriodic(1s);
  timerOnce->startOnce(5s);
}
