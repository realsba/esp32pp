#ifndef ESP32PP_RF433_HPP
#define ESP32PP_RF433_HPP

#include <chrono>

#include <driver/rmt_encoder.h>

namespace esp32pp {

class RF433
{
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

  enum class State
  {
    SendData,
    SendResetCode
  };

  size_t encode(rmt_channel_handle_t channel, const void* data, size_t dataSize, rmt_encode_state_t* retState);
  esp_err_t reset();

  rmt_encoder_t m_encoder {.encode = encode, .reset = reset, .del = nullptr};
  rmt_channel_handle_t m_channel {nullptr};
  rmt_encoder_handle_t m_bytesEncoder {nullptr};
  rmt_encoder_handle_t m_copyEncoder {nullptr};
  rmt_symbol_word_t m_resetCode {};
  State m_state {State::SendData};
};

} // namespace esp32pp

#endif // ESP32PP_RF433_HPP
