#include "RF433.hpp"

#include <driver/rmt_tx.h>

#include <esp_log.h>
#include <esp_attr.h>

using namespace std::chrono_literals;

void RF433::setup(uint8_t gpio)
{
  constexpr const uint32_t clockResolution {10000000};
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
      .io_od_mode = false
    }
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &m_channel));

  rmt_bytes_encoder_config_t bytesEncoderConfig;
  bytesEncoderConfig.bit0.duration0 = std::chrono::duration_cast<Duration>(3500ns).count();
  bytesEncoderConfig.bit0.level0 = 1;
  bytesEncoderConfig.bit0.duration1 = std::chrono::duration_cast<Duration>(8000ns).count();
  bytesEncoderConfig.bit0.level1 = 0;
  bytesEncoderConfig.bit1.duration0 = std::chrono::duration_cast<Duration>(7000ns).count();
  bytesEncoderConfig.bit1.level0 = 1;
  bytesEncoderConfig.bit1.duration1 = std::chrono::duration_cast<Duration>(6000ns).count();
  bytesEncoderConfig.bit1.level1 = 0;
  bytesEncoderConfig.flags.msb_first = true;
  ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytesEncoderConfig, &m_bytesEncoder));

  rmt_copy_encoder_config_t copyEncoderConfig = {};
  ESP_ERROR_CHECK(rmt_new_copy_encoder(&copyEncoderConfig, &m_copyEncoder));

  auto ticks = std::chrono::duration_cast<Duration>(50us).count() / 2;
  m_resetCode.duration0 = ticks;
  m_resetCode.level0 = 0;
  m_resetCode.duration1 = ticks;
  m_resetCode.level1 = 0;

  rmt_enable(m_channel);
}

RF433::~RF433()
{
  rmt_del_channel(m_channel);
  rmt_del_encoder(m_bytesEncoder);
  rmt_del_encoder(m_copyEncoder);
}

bool RF433::send(const void* data, size_t dataSize)
{
  rmt_transmit_config_t config = {
    .loop_count = 0,
    .flags = {
      .eot_level = 0,
      .queue_nonblocking = 0
    }
  };
  return rmt_transmit(m_channel, &m_encoder, data, dataSize, &config) == ESP_OK;
}

size_t IRAM_ATTR RF433::encode(
  rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState
)
{
  return reinterpret_cast<RF433*>(encoder)->encode(channel, data, dataSize, retState);
}

esp_err_t IRAM_ATTR RF433::reset(rmt_encoder_t* encoder)
{
  return reinterpret_cast<RF433*>(encoder)->reset();
}

size_t IRAM_ATTR RF433::encode(
  rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState
)
{
  rmt_encode_state_t sessionState {};
  int state = 0;
  size_t encodedSymbols = 0;

  switch (m_state) {
    case State::SendData:
      encodedSymbols += m_bytesEncoder->encode(m_bytesEncoder, channel, data, dataSize, &sessionState);
      if (sessionState & RMT_ENCODING_COMPLETE) {
        m_state = State::SendResetCode;
      }
      if (sessionState & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        break;
      }
      // fall-through
    case State::SendResetCode:
      encodedSymbols += m_copyEncoder->encode(m_copyEncoder, channel, &m_resetCode, sizeof(m_resetCode), &sessionState);
      if (sessionState & RMT_ENCODING_COMPLETE) {
        m_state = State::SendData;
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

esp_err_t IRAM_ATTR RF433::reset()
{
  rmt_encoder_reset(m_bytesEncoder);
  rmt_encoder_reset(m_copyEncoder);
  m_state = State::SendData;
  return ESP_OK;
}
