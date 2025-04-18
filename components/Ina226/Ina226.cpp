// file   : Ina226.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Ina226.hpp"
#include "I2CBus.hpp"

#include "esp_log.h"

namespace esp32pp {

std::string Ina226::toString(Register reg)
{
    switch (reg) {
        case Register::Configuration: return "Configuration";
        case Register::ShuntVoltage: return "ShuntVoltage";
        case Register::BusVoltage: return "BusVoltage";
        case Register::Power: return "Power";
        case Register::Current: return "Current";
        case Register::Calibration: return "Calibration";
        case Register::Mask: return "Mask";
        case Register::AlertLimit: return "AlertLimit";
        case Register::ManufacturerID: return "ManufacturerID";
        case Register::DieID: return "DieID";
        default: return "Unknown Register";
    }
}

Ina226::Ina226(I2CBus& bus, uint16_t address)
    : _bus(bus)
    , _address(address)
{
    _bus.addDevice(address, &_handle);
}

void Ina226::calibrate(float shuntResistor, float maxCurrent)
{
    _currentLSB = maxCurrent / 32768;
    _powerLSB = 25 * _currentLSB;
    auto calibrationValue = static_cast<uint16_t>(0.00512 / (_currentLSB * shuntResistor));
    writeRegister(Register::Calibration, calibrationValue);
}

uint16_t Ina226::readRegister(Register reg)
{
    auto writeData = static_cast<uint8_t>(reg);
    uint8_t readData[2];

    esp_err_t err = i2c_master_transmit_receive(
        _handle, &writeData, sizeof(writeData), readData, sizeof(readData), 1000
    );
    if (err != ESP_OK) {
        ESP_LOGE("INA226", "Failed to read register %s (0x%02x): %s", toString(reg).c_str(), static_cast<uint8_t>(reg),
            esp_err_to_name(err));
        return 0;
    }

    return (readData[0] << 8) | readData[1];
}

void Ina226::writeRegister(Register reg, uint16_t value)
{
    uint8_t data[3];
    data[0] = static_cast<uint8_t>(reg);
    data[1] = (value >> 8) & 0xFF;
    data[2] = value & 0xFF;

    esp_err_t err = i2c_master_transmit(_handle, data, sizeof(data), 1000);
    if (err != ESP_OK) {
        ESP_LOGE("INA226", "Failed to write register %s (0x%02x): %s", toString(reg).c_str(), static_cast<uint8_t>(reg),
            esp_err_to_name(err));
    }
}

float Ina226::getShuntVoltage()
{
    auto raw = readRegister(Register::ShuntVoltage);
    return static_cast<float>(raw * 2.5e-6);
}

float Ina226::getBusVoltage()
{
    auto raw = readRegister(Register::BusVoltage);
    return static_cast<float>(raw * 1.25e-3);
}

float Ina226::getPower()
{
    auto raw = readRegister(Register::Power);
    return raw * _powerLSB;
}

float Ina226::getCurrent()
{
    auto raw = readRegister(Register::Current);
    return raw * _currentLSB;
}

} // namespace esp32pp
