#ifndef ESP32PP_ACTIONSEQUENCE_HPP
#define ESP32PP_ACTIONSEQUENCE_HPP

#include <asio.hpp>
#include <vector>
#include <functional>
#include <chrono>
#include <utility>
#include <cstddef>

namespace esp32pp {

class ActionSequence {
public:
    enum class RepeatMode {
        None,
        Count,
        Duration,
        Infinite
    };

    using Action = std::pair<std::function<void()>, std::chrono::milliseconds>;

    explicit ActionSequence(asio::io_context& ioc);

    template <typename Func>
    void addAction(Func&& action, std::chrono::milliseconds duration);

    void setNoRepeat();
    void setRepeatCount(uint32_t count);
    void setRepeatDuration(std::chrono::milliseconds duration);
    void setInfiniteRepeat();

    void setOnComplete(std::function<void()> callback);

    void start();

private:
    void executeNextAction();

    asio::steady_timer _timer;
    std::function<void()> _onComplete;
    std::vector<Action> _actions;
    size_t _currentActionIndex {0};

    std::chrono::milliseconds _repeatDuration {0};
    std::chrono::steady_clock::time_point _startTime;
    RepeatMode _repeatMode {RepeatMode::None};
    uint32_t _repeatCount {0};
    uint32_t _executedRepeats {0};
};

template <typename Func>
void ActionSequence::addAction(Func&& action, std::chrono::milliseconds duration)
{
    _actions.emplace_back([func = std::forward<Func>(action)] { func(); }, duration);
}

} // namespace esp32pp

#endif // ESP32PP_ACTIONSEQUENCE_HPP
