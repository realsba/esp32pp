// file   : ActionSequence.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include "RepeatStrategy.hpp"

#include <asio.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace esp32pp {

class ActionSequence {
public:
    using Action = std::pair<std::function<void()>, std::chrono::milliseconds>;

    explicit ActionSequence(asio::any_io_executor executor);

    template <typename Func>
    void addAction(Func&& action, std::chrono::milliseconds duration);

    void setNoRepeat();
    void setRepeatCount(uint32_t count);
    void setRepeatDuration(std::chrono::milliseconds duration);
    void setInfiniteRepeat();

    void setOnComplete(std::function<void()> callback);

    void start();
    void stop();

private:
    void executeNextAction();
    void complete();

    asio::any_io_executor _executor;
    asio::steady_timer _timer;
    std::function<void()> _onComplete;
    std::vector<Action> _actions;
    RepeatStrategyPtr _repeatStrategy {std::make_unique<RepeatNone>()};
    size_t _currentActionIndex {0};
    bool _running {false};
};

template <typename Func>
void ActionSequence::addAction(Func&& action, std::chrono::milliseconds duration)
{
    _actions.emplace_back([func = std::forward<Func>(action)] { func(); }, duration);
}

} // namespace esp32pp
