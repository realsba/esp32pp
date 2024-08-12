#ifndef INA226_HPP
#define INA226_HPP

#include <string>

#include "driver/i2c_master.h"

class I2CBus;

class Ina226
{
public:
  enum class Register : uint8_t {
    Configuration   = 0x00,
    ShuntVoltage    = 0x01,
    BusVoltage      = 0x02,
    Power           = 0x03,
    Current         = 0x04,
    Calibration     = 0x05,
    Mask            = 0x06,
    AlertLimit      = 0x07,
    ManufacturerID  = 0xFE,
    DieID           = 0xFF,
  };

  static std::string toString(Register reg);

  Ina226(I2CBus& bus, uint16_t address);

  void calibrate(float shuntResistor, float maxCurrent);

  uint16_t readRegister(Register reg);
  void writeRegister(Register reg, uint16_t value);

  float getShuntVoltage();
  float getBusVoltage();
  float getPower();
  float getCurrent();

private:
  I2CBus&                       m_bus;
  uint16_t                      m_address;
  i2c_master_dev_handle_t       m_handle {nullptr};
  double                        m_shuntResistorValue {0.1};
  double                        m_currentLSB {0};
  double                        m_powerLSB {0};};

#endif // INA226_HPP
