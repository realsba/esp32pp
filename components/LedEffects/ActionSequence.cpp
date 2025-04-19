// file   : ActionSequence.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "ActionSequence.hpp"

namespace esp32pp {

ActionSequence::ActionSequence(asio::any_io_executor executor)
    : _executor(std::move(executor))
    , _timer(_executor)
{}

void ActionSequence::setNoRepeat()
{
    _repeatStrategy = std::make_unique<RepeatNone>();
}

void ActionSequence::setRepeatCount(uint32_t count)
{
    _repeatStrategy = std::make_unique<RepeatCount>(count);
}

void ActionSequence::setRepeatDuration(std::chrono::milliseconds duration)
{
    _repeatStrategy = std::make_unique<RepeatDuration>(duration);
}

void ActionSequence::setInfiniteRepeat()
{
    _repeatStrategy = std::make_unique<RepeatInfinite>();
}

void ActionSequence::setOnComplete(std::function<void()> callback)
{
    _onComplete = std::move(callback);
}

void ActionSequence::start()
{
    asio::post(_executor, [this] {
        if (_running) return;

        if (_actions.empty()) {
            complete();
            return;
        }

        _running = true;
        _currentActionIndex = 0;
        _repeatStrategy->reset();

        executeNextAction();
    });
}

void ActionSequence::stop()
{
    asio::post(_executor, [this] {
        _running = false;
        _timer.cancel();
    });
}

void ActionSequence::executeNextAction()
{
    if (!_running) return;

    if (_currentActionIndex >= _actions.size()) {
        if (_repeatStrategy->shouldRepeat(_actions.size())) {
            _currentActionIndex = 0;
        } else {
            _running = false;
            complete();
            return;
        }
    }

    auto& [action, duration] = _actions[_currentActionIndex];
    action();

    _timer.expires_after(duration);
    _timer.async_wait(
        [this](const asio::error_code& ec) {
            if (!ec) {
                ++_currentActionIndex;
                executeNextAction();
            }
        }
    );
}

void ActionSequence::complete()
{
    if (_onComplete) {
        asio::post(_executor, _onComplete);
    }
}

} // namespace esp32pp
