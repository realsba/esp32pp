// file   : RepeatStrategy.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <chrono>
#include <memory>

namespace esp32pp {

class RepeatStrategy {
public:
    virtual ~RepeatStrategy() = default;

    virtual void reset() = 0;
    virtual bool shouldRepeat(size_t actionCount) = 0;
};

using RepeatStrategyPtr = std::unique_ptr<RepeatStrategy>;

class RepeatNone final : public RepeatStrategy {
public:
    void reset() override {}
    bool shouldRepeat(size_t actionCount) override { return false; }
};

class RepeatInfinite final : public RepeatStrategy {
public:
    void reset() override {}
    bool shouldRepeat(size_t) override { return true; }
};

class RepeatCount final : public RepeatStrategy {
public:
    explicit RepeatCount(uint32_t count) : _count(count) {}

    void reset() override { _executed = 0; }
    bool shouldRepeat(size_t) override { return ++_executed < _count; }

private:
    uint32_t _count;
    uint32_t _executed = 0;
};

class RepeatDuration final : public RepeatStrategy {
public:
    explicit RepeatDuration(std::chrono::milliseconds duration)
        : _duration(duration) {}

    void reset() override { _startTime = std::chrono::steady_clock::now(); }
    bool shouldRepeat(size_t) override { return std::chrono::steady_clock::now() - _startTime < _duration; }

private:
    std::chrono::milliseconds _duration;
    std::chrono::steady_clock::time_point _startTime;
};

} // namespace esp32pp
