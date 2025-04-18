// file   : RF433.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <chrono>

#include <driver/rmt_encoder.h>

namespace esp32pp {

class RF433 {
public:
    ~RF433();

    void setup(uint8_t gpio);
    bool send(const void* data, size_t dataSize);

private:
    static size_t encode(
        rmt_encoder_t* encoder,
        rmt_channel_handle_t channel,
        const void* data, size_t dataSize,
        rmt_encode_state_t* retState
    );
    static esp_err_t reset(rmt_encoder_t* encoder);

    enum class State {
        SendData,
        SendResetCode
    };

    size_t encode(rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState);
    esp_err_t reset();

    rmt_encoder_t _encoder{.encode = encode, .reset = reset, .del = nullptr};
    rmt_channel_handle_t _channel{nullptr};
    rmt_encoder_handle_t _bytesEncoder{nullptr};
    rmt_encoder_handle_t _copyEncoder{nullptr};
    rmt_symbol_word_t _resetCode{};
    State _state{State::SendData};
};

} // namespace esp32pp
