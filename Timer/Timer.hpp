#ifndef ESP32PP_TIMER_HPP
#define ESP32PP_TIMER_HPP

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

    // @formatter:off
    esp_timer_handle_t          m_timerHandle {};
    Callback                    m_callback;
    bool                        m_isRunning {false};
    // @formatter:on
};

} // namespace esp32pp

#endif // ESP32PP_TIMER_HPP
