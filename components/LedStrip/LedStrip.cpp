// file   : LedStrip.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "LedStrip.hpp"

#include <driver/rmt_tx.h>

#include <esp_log.h>
#include <esp_attr.h>

namespace esp32pp {

LedStripConfig::LedStripConfig(
    std::chrono::nanoseconds t0h,
    std::chrono::nanoseconds t1h,
    std::chrono::nanoseconds t0l,
    std::chrono::nanoseconds t1l,
    std::chrono::nanoseconds rst
)
    : T0H(t0h), T1H(t1h), T0L(t0l), T1L(t1l), RST(rst)
{}

void LedStrip::setup(uint8_t gpio, const LedStripConfig& config)
{
    constexpr uint32_t clockResolution{10000000};
    using Duration = std::chrono::duration<int64_t, std::ratio<1, clockResolution>>;

    rmt_tx_channel_config_t channelConfig = {
        .gpio_num = static_cast<gpio_num_t>(gpio),
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = clockResolution,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
        .intr_priority = 0,
        .flags = {
            .invert_out = false,
            .with_dma = false,
            .io_loop_back = false,
            .io_od_mode = false,
            .allow_pd = false
        }
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &_channel));

    rmt_bytes_encoder_config_t bytesEncoderConfig;
    bytesEncoderConfig.bit0.duration0 = std::chrono::duration_cast<Duration>(config.T0H).count();
    bytesEncoderConfig.bit0.level0 = 1;
    bytesEncoderConfig.bit0.duration1 = std::chrono::duration_cast<Duration>(config.T0L).count();
    bytesEncoderConfig.bit0.level1 = 0;
    bytesEncoderConfig.bit1.duration0 = std::chrono::duration_cast<Duration>(config.T1H).count();
    bytesEncoderConfig.bit1.level0 = 1;
    bytesEncoderConfig.bit1.duration1 = std::chrono::duration_cast<Duration>(config.T1L).count();
    bytesEncoderConfig.bit1.level1 = 0;
    bytesEncoderConfig.flags.msb_first = true;
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytesEncoderConfig, &_bytesEncoder));

    rmt_copy_encoder_config_t copyEncoderConfig = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copyEncoderConfig, &_copyEncoder));

    auto ticks = std::chrono::duration_cast<Duration>(config.RST).count() / 2;
    _resetCode.duration0 = ticks;
    _resetCode.level0 = 0;
    _resetCode.duration1 = ticks;
    _resetCode.level1 = 0;

    rmt_enable(_channel);
}

LedStrip::~LedStrip()
{
    rmt_del_channel(_channel);
    rmt_del_encoder(_bytesEncoder);
    rmt_del_encoder(_copyEncoder);
}

bool LedStrip::transmit(const void* data, size_t dataSize)
{
    rmt_transmit_config_t config = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = 0
        }
    };
    return rmt_transmit(_channel, &_encoder, data, dataSize, &config) == ESP_OK;
}

size_t IRAM_ATTR LedStrip::encode_led_strip(
    rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* data, size_t dataSize,
    rmt_encode_state_t* retState
)
{
    return reinterpret_cast<LedStrip*>(encoder)->encode(channel, data, dataSize, retState);
}

esp_err_t IRAM_ATTR LedStrip::reset_led_strip(rmt_encoder_t* encoder)
{
    return reinterpret_cast<LedStrip*>(encoder)->reset();
}

size_t IRAM_ATTR LedStrip::encode(
    rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState
)
{
    rmt_encode_state_t sessionState{};
    int state = 0;
    size_t encodedSymbols = 0;

    switch (_state) {
        case State::SendRgbData:
            encodedSymbols += _bytesEncoder->encode(_bytesEncoder, channel, data, dataSize, &sessionState);
            if (sessionState & RMT_ENCODING_COMPLETE) {
                _state = State::SendResetCode;
            }
            if (sessionState & RMT_ENCODING_MEM_FULL) {
                state |= RMT_ENCODING_MEM_FULL;
                break;
            }
        // fall-through
        case State::SendResetCode:
            encodedSymbols += _copyEncoder->encode(_copyEncoder, channel, &_resetCode, sizeof(_resetCode),
                &sessionState);
            if (sessionState & RMT_ENCODING_COMPLETE) {
                _state = State::SendRgbData;
                state |= RMT_ENCODING_COMPLETE;
            }
            if (sessionState & RMT_ENCODING_MEM_FULL) {
                state |= RMT_ENCODING_MEM_FULL;
                break;
            }
    }
    *retState = static_cast<rmt_encode_state_t>(state);
    return encodedSymbols;
}

esp_err_t IRAM_ATTR LedStrip::reset()
{
    rmt_encoder_reset(_bytesEncoder);
    rmt_encoder_reset(_copyEncoder);
    _state = State::SendRgbData;
    return ESP_OK;
}

} // namespace esp32pp
