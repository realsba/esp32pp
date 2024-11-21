#include "ActionSequence.hpp"

namespace esp32pp {

ActionSequence::ActionSequence(asio::io_context& ioc)
    : _timer(ioc)
{
}

void ActionSequence::setNoRepeat()
{
    _repeatMode     = RepeatMode::None;
    _repeatCount    = 0;
    _repeatDuration = std::chrono::milliseconds(0);
}

void ActionSequence::setRepeatCount(uint32_t count)
{
    _repeatMode     = RepeatMode::Count;
    _repeatCount    = count;
    _repeatDuration = std::chrono::milliseconds(0);
}

void ActionSequence::setRepeatDuration(std::chrono::milliseconds duration)
{
    _repeatMode     = RepeatMode::Duration;
    _repeatDuration = duration;
    _repeatCount    = 0;
}

void ActionSequence::setInfiniteRepeat()
{
    _repeatMode     = RepeatMode::Infinite;
    _repeatCount    = 0;
    _repeatDuration = std::chrono::milliseconds(0);
}

void ActionSequence::setOnComplete(std::function<void()> callback)
{
    _onComplete = std::move(callback);
}

void ActionSequence::start()
{
    if (_actions.empty() && _repeatCount == 0 && _repeatDuration.count() == 0) {
        if (_onComplete) {
            _onComplete();
        }
        return;
    }

    _executedRepeats    = 0;
    _currentActionIndex = 0;
    _startTime          = std::chrono::steady_clock::now();
    executeNextAction();
}

void ActionSequence::executeNextAction()
{
    if (_currentActionIndex >= _actions.size()) {
        _currentActionIndex = 0;

        if (_repeatMode == RepeatMode::Count) {
            if (++_executedRepeats >= _repeatCount) {
                if (_onComplete) {
                    _onComplete();
                }
                return;
            }
        } else if (_repeatMode == RepeatMode::Duration) {
            auto elapsed = std::chrono::steady_clock::now() - _startTime;
            if (elapsed >= _repeatDuration) {
                if (_onComplete) {
                    _onComplete();
                }
                return;
            }
        }

    }

    auto& [action, duration] = _actions[_currentActionIndex];
    action();

    _timer.expires_after(duration);
    _timer.async_wait(
        [this](const asio::error_code&)
        {
            ++_currentActionIndex;
            executeNextAction();
        }
    );
}

} // namespace esp32pp
