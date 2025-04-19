// file   : I2CBus.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <driver/i2c_master.h>

namespace esp32pp {

class I2CBus {
public:
    I2CBus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, uint32_t freq);
    ~I2CBus();

    esp_err_t addDevice(uint16_t address, i2c_master_dev_handle_t* deviceHandle);

private:
    void init();
    void deinit();

    i2c_port_t _port;
    gpio_num_t _sda;
    gpio_num_t _scl;
    uint32_t _freq;
    i2c_master_bus_handle_t _busHandle{nullptr};
};

} // namespace esp32pp
