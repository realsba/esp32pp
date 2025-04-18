// file   : Timer.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <esp_timer.h>

#include <functional>
#include <chrono>

namespace esp32pp {

class Timer {
public:
    using Callback = std::function<void()>;
    using Duration = std::chrono::microseconds;

    explicit Timer(Callback callback);
    ~Timer();

    void startPeriodic(Duration interval);
    void startOnce(Duration timeout);
    void stop();
    bool isRunning() const;

private:
    static void timerCallback(void* arg);

    esp_timer_handle_t _timerHandle{};
    Callback _callback;
    bool _isRunning{false};
};

} // namespace esp32pp
