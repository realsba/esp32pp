// file   : Timer.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Timer.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "Timer";

Timer::Timer(Callback callback)
    : _callback(std::move(callback))
{
    esp_timer_create_args_t timerArgs = {
        .callback = &Timer::timerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "Timer",
        .skip_unhandled_events = true
    };

    auto err = esp_timer_create(&timerArgs, &_timerHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer: %s", esp_err_to_name(err));
    }
}

Timer::~Timer()
{
    stop();
    if (_timerHandle) {
        esp_timer_delete(_timerHandle);
    }
}

void Timer::startPeriodic(Duration interval)
{
    if (_timerHandle && !_isRunning) {
        ESP_ERROR_CHECK(esp_timer_start_periodic(_timerHandle, interval.count()));
        _isRunning = true;
    }
}

void Timer::startOnce(Duration timeout)
{
    if (_timerHandle && !_isRunning) {
        ESP_ERROR_CHECK(esp_timer_start_once(_timerHandle, timeout.count()));
        _isRunning = true;
    }
}

void Timer::stop()
{
    if (_timerHandle && _isRunning) {
        esp_timer_stop(_timerHandle);
        _isRunning = false;
    }
}

bool Timer::isRunning() const
{
    return _isRunning;
}

void Timer::timerCallback(void* arg)
{
    auto* timer = static_cast<Timer*>(arg);
    if (timer && timer->_callback) {
        timer->_callback();
    }
}

} // esp32pp
