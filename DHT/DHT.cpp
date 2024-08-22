#include "DHT.hpp"

#include <driver/gpio.h>
#include <driver/rmt_rx.h>

#include <esp_log.h>

#include <limits>

namespace esp32pp {

constexpr auto TAG = "DHT";

uint32_t DHT::s_nextId {0};

DHT::~DHT()
{
  if (m_channel) {
    rmt_del_channel(m_channel);
  }
}

void DHT::setup(gpio_num_t gpio, uint32_t clockResolution)
{
  m_pin = gpio;

  ESP_LOGI(TAG, "Create RMT RX channel");
  rmt_rx_channel_config_t channelConfig = {
    .gpio_num = gpio,
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = clockResolution,
    .mem_block_symbols = 64,
    .intr_priority = 0,
    .flags = {
      .invert_in = false,
      .with_dma = false,
      .io_loop_back = false,
      .backup_before_sleep = true
    }
  };
  ESP_ERROR_CHECK(rmt_new_rx_channel(&channelConfig, &m_channel));

  ESP_LOGI(TAG, "Register RX done callback");
  rmt_rx_event_callbacks_t cbs = {
    .on_recv_done = rx_done_callback,
  };

  ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(m_channel, &cbs, m_task.getNativeHandle()));

  gpio_set_direction(m_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(m_pin, 1);
}

void DHT::read(Handler&& handler)
{
  const static rmt_receive_config_t config = {
    .signal_range_min_ns = 5000,
    .signal_range_max_ns = 120000,
    .flags = {.en_partial_rx = 0}
  };

  m_handler = std::move(handler);
  m_status = std::error_code();

  gpio_set_direction(m_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(m_pin, 0);
  esp_rom_delay_us(m_startSignalDuration);
  rmt_enable(m_channel);
  ESP_ERROR_CHECK(rmt_receive(m_channel, m_symbols, sizeof(m_symbols), &config));
  gpio_set_direction(m_pin, GPIO_MODE_INPUT);
}

bool IRAM_ATTR DHT::rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* eventData, void* context)
{
  BaseType_t highTaskWakeup = pdFALSE;
  auto taskToNotify = static_cast<TaskHandle_t>(context);
  xTaskNotifyFromISR(taskToNotify, reinterpret_cast<uint32_t>(eventData), eSetValueWithOverwrite, &highTaskWakeup);
  return highTaskWakeup == pdTRUE;
}

void DHT::readSensor()
{
  rmt_rx_done_event_data_t* eventData {nullptr};

  if (xTaskNotifyWait(0x00, ULONG_MAX, reinterpret_cast<uint32_t*>(&eventData), portMAX_DELAY) == pdTRUE) {
    ESP_LOGI(TAG, "Decode a response %p %d", eventData->received_symbols, eventData->num_symbols);

    decode(eventData->received_symbols, eventData->num_symbols);

    rmt_disable(m_channel);
    gpio_set_direction(m_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(m_pin, 1);

    if (m_handler) {
      m_handler(m_status, humidity(), temperature());
    }
  }
}

void DHT::decode(rmt_symbol_word_t* data, size_t numItems)
{
  m_data[0] = 0;
  m_data[1] = 0;
  m_data[2] = 0;
  m_data[3] = 0;
  m_data[4] = 0;
  m_status = error::DhtCategory::BadResponse;

  if (numItems < 42) {
    ESP_LOGD(TAG, "Incorrect pulse count: %u", numItems);
    return;
  }

  auto* ptr = reinterpret_cast<rmt_symbol_word_t*>(reinterpret_cast<uint8_t*>(data) + 2);
  --numItems;

  auto duration = ptr[0].duration0;
  if (duration < 75 || duration > 180) {
    ESP_LOGD(TAG, "Bad response signal. Low time: %u", duration);
    return;
  }
  duration = ptr[0].duration1;
  if (duration < 75 || duration > 180) {
    ESP_LOGD(TAG, "Bad response signal. High time: %u", duration);
    return;
  }

  for (int i = 1; i < numItems - 1; ++i) {
    if (ptr[i].level0 != 0 || ptr[i].level1 != 1) {
      ESP_LOGD(TAG, "Incorrect level. item: %d, level0: %u, level1: %u", i, ptr[i].level0, ptr[i].level1);
      return;
    }

    duration = ptr[i].duration0;
    if (duration < 48 || duration > 55) {
      ESP_LOGE(TAG, "Incorrect duration0. item: %d, duration: %u", i, duration);
      return;
    }
    m_data[(i - 1) / 8] <<= 1;
    duration = ptr[i].duration1;
    if (duration >= 68 && duration <= 75) {
      m_data[(i - 1) / 8] |= 1;
    } else if (duration < 22 || duration > 30) {
      ESP_LOGE(TAG, "Incorrect duration1. item: %d, duration: %u", i, duration);
      return;
    }
  }

  if (m_data[4] == ((m_data[0] + m_data[1] + m_data[2] + m_data[3]) & 0xFF)) {
    m_status = std::error_code();
  }
}

DHT11::DHT11()
{
  m_startSignalDuration = 30000;
}

float DHT11::temperature()
{
  if (m_status) {
    return std::numeric_limits<float>::quiet_NaN();
  }
  return static_cast<float>(m_data[2]);
}

float DHT11::humidity()
{
  if (m_status) {
    return std::numeric_limits<float>::quiet_NaN();
  }
  return static_cast<float>(m_data[0]);
}

float DHT22::temperature()
{
  if (m_status) {
    return std::numeric_limits<float>::quiet_NaN();
  }
  auto temp = static_cast<float>((((m_data[2] & 0x7F) << 8) | m_data[3]) * 0.1);
  if (m_data[2] & 0x80) {
    temp = -temp;
  }
  return temp;
}

float DHT22::humidity()
{
  if (m_status) {
    return std::numeric_limits<float>::quiet_NaN();
  }
  return static_cast<float>(((m_data[0] << 8) | m_data[1]) * 0.1);
}

namespace error {
namespace detail {

const std::error_category& DhtCategory::getInstance()
{
  static const DhtCategory instance;
  return instance;
}

const char* DhtCategory::name() const noexcept
{
  return "DHT";
}

std::string DhtCategory::message(int ev) const
{
  using category = error::DhtCategory;

  switch (static_cast<category>(ev)) {
    case category::BadResponse: return "Bad response from sensor";
  }

  return "Undefined error";
}

} // namespace detail
} // namespace error
} // namespace esp32pp