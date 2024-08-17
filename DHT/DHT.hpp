#ifndef ESP32PP_DHT_HPP
#define ESP32PP_DHT_HPP

#include "Task.hpp"

#include <freertos/FreeRTOS.h>

#include <driver/rmt_types.h>
#include <driver/gpio.h>

#include <functional>
#include <system_error>

class DHT
{
public:
  using Handler = std::function<void(std::error_code ec, float humidity, float temperature)>;

  virtual ~DHT();

  void setup(gpio_num_t gpio, uint32_t clockResolution);
  void read(Handler&& handler);

private:
  static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* eventData, void* context);

  void readSensor();
  void decode(rmt_symbol_word_t* data, size_t numItems);

  virtual float temperature() = 0;
  virtual float humidity() = 0;

protected:
  std::error_code m_status;
  uint16_t m_startSignalDuration {1000};
  uint8_t m_data[5] {};

private:
  static uint32_t s_nextId;

  uint32_t m_id = s_nextId;
  rmt_symbol_word_t m_symbols[64];
  Task m_task {[this] { readSensor(); }, "DHT:" + std::to_string(m_id)};
  Handler m_handler;
  rmt_channel_handle_t m_channel {nullptr};
  gpio_num_t m_pin {GPIO_NUM_0};
};

class DHT11 : public DHT
{
public:
  explicit DHT11();

private:
  float temperature() override;
  float humidity() override;
};

class DHT22 : public DHT
{
  float temperature() override;
  float humidity() override;
};

namespace error {

enum class DhtCategory
{
  BadResponse = 1
};

namespace detail {

class DhtCategory : public std::error_category
{
public:
  DhtCategory() = default;

  static const std::error_category& getInstance();

  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

} // namespace detail

inline std::error_code make_error_code(DhtCategory e)
{
  return {static_cast<int>(e), detail::DhtCategory::getInstance()};
}

} // namespace error

namespace std {

template<>
struct is_error_code_enum<error::DhtCategory> : public true_type {};

} // namespace std

#endif // ESP32PP_DHT_HPP
