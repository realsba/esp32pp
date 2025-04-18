// file   : LedStrip.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <chrono>

#include <driver/rmt_encoder.h>

namespace esp32pp {

struct LedStripConfig {
    LedStripConfig(
        std::chrono::nanoseconds t0h,
        std::chrono::nanoseconds t1h,
        std::chrono::nanoseconds t0l,
        std::chrono::nanoseconds t1l,
        std::chrono::nanoseconds rst
    );

    std::chrono::nanoseconds T0H;
    std::chrono::nanoseconds T1H;
    std::chrono::nanoseconds T0L;
    std::chrono::nanoseconds T1L;
    std::chrono::nanoseconds RST;
};

using namespace std::chrono_literals;

struct WS2812Config : LedStripConfig {
    WS2812Config() : LedStripConfig(350ns, 700ns, 800ns, 600ns, 50us)
    {}
};

struct WS2812BConfig : LedStripConfig {
    WS2812BConfig() : LedStripConfig(400ns, 800ns, 850ns, 450ns, 50us)
    {}
};

struct SK6812Config : LedStripConfig {
    SK6812Config() : LedStripConfig(300ns, 600ns, 900ns, 600ns, 80us)
    {}
};

class LedStrip {
public:
    ~LedStrip();

    void setup(uint8_t gpio, const LedStripConfig& config);
    bool transmit(const void* data, size_t dataSize);

private:
    static size_t encode_led_strip(
        rmt_encoder_t* encoder,
        rmt_channel_handle_t channel,
        const void* data, size_t dataSize,
        rmt_encode_state_t* retState
    );
    static esp_err_t reset_led_strip(rmt_encoder_t* encoder);

    enum class State {
        SendRgbData,
        SendResetCode
    };

    size_t encode(rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState);
    esp_err_t reset();

    rmt_encoder_t _encoder{.encode = encode_led_strip, .reset = reset_led_strip, .del = nullptr};
    rmt_channel_handle_t _channel{nullptr};
    rmt_encoder_handle_t _bytesEncoder{nullptr};
    rmt_encoder_handle_t _copyEncoder{nullptr};
    rmt_symbol_word_t _resetCode{};
    State _state{State::SendRgbData};
};

} // namespace esp32pp
