#ifndef ESP32PP_I2CBUS_HPP
#define ESP32PP_I2CBUS_HPP

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

    // @formatter:off
    i2c_port_t                  m_port;
    gpio_num_t                  m_sda;
    gpio_num_t                  m_scl;
    uint32_t                    m_freq;
    i2c_master_bus_handle_t     m_busHandle {nullptr};
    // @formatter:on
};

} // namespace esp32pp

#endif // ESP32PP_I2CBUS_HPP
