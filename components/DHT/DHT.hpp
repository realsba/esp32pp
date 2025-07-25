// file   : DHT.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include "Task.hpp"

#include <freertos/FreeRTOS.h>

#include <driver/rmt_types.h>
#include <driver/gpio.h>

#include <functional>
#include <system_error>

namespace esp32pp {

class DHT {
public:
    using Handler = std::function<void(std::error_code ec, float humidity, float temperature)>;

    virtual ~DHT();

    void setup(gpio_num_t gpio, uint32_t clockResolution);
    void read(Handler handler);

private:
    static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* eventData,
        void* context);

    void readSensor();
    void decode(rmt_symbol_word_t* data, size_t numItems);

    virtual float temperature() = 0;
    virtual float humidity() = 0;

protected:
    std::error_code _status;
    uint16_t _startSignalDuration{1000};
    uint8_t _data[5]{};

private:
    static uint32_t s_nextId;

    uint32_t _id = s_nextId;
    rmt_symbol_word_t _symbols[64]{};
    Task _task{[this] { readSensor(); }, "DHT:" + std::to_string(_id)};
    Handler _readHandler;
    rmt_channel_handle_t _channel{nullptr};
    gpio_num_t _pin{GPIO_NUM_0};
};

class DHT11 final : public DHT {
public:
    explicit DHT11();

private:
    float temperature() override;
    float humidity() override;
};

class DHT22 final : public DHT {
    float temperature() override;
    float humidity() override;
};

namespace error {

enum class DhtCategory {
    BadResponse = 1
};

namespace detail {

class DhtCategory : public std::error_category {
public:
    DhtCategory() = default;

    static const std::error_category& getInstance();

    [[nodiscard]] const char* name() const noexcept override;
    [[nodiscard]] std::string message(int ev) const override;
};

} // namespace detail

inline std::error_code make_error_code(DhtCategory e)
{
    return {static_cast<int>(e), detail::DhtCategory::getInstance()};
}

} // namespace error

} // namespace esp32pp

template <>
struct std::is_error_code_enum<esp32pp::error::DhtCategory> : true_type {};
