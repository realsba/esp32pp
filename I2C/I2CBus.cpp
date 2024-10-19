#include "I2CBus.hpp"

#include "esp_err.h"
#include "esp_log.h"

namespace esp32pp {

I2CBus::I2CBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, uint32_t freq)
    : _port(port)
    , _sda(sda)
    , _scl(scl)
    , _freq(freq)
{
    init();
}

I2CBus::~I2CBus()
{
    deinit();
}

esp_err_t I2CBus::addDevice(uint16_t address, i2c_master_dev_handle_t* deviceHandle)
{
    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = _freq,
        .scl_wait_us = 0,
        .flags = {.disable_ack_check = 0},
    };

    return i2c_master_bus_add_device(_busHandle, &config, deviceHandle);
}

void I2CBus::init()
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = _port,
        .sda_io_num = _sda,
        .scl_io_num = _scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {.enable_internal_pullup = false},
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &_busHandle);
    if (err != ESP_OK) {
        ESP_LOGE("I2CBus", "Failed to initialize I2C: %s", esp_err_to_name(err));
    }
}

void I2CBus::deinit()
{
    if (_busHandle) {
        i2c_del_master_bus(_busHandle);
    }
}

} // namespace esp32pp
