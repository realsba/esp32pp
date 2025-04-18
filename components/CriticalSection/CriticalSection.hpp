// file   : CriticalSection.hpp
// author : sba <bohdan.sadovyak@gmail.com>

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

namespace esp32pp {

class CriticalSection {
public:
    explicit CriticalSection(portMUX_TYPE& mux);
    ~CriticalSection();

private:
    portMUX_TYPE& _mux;
};

class CriticalSectionIsr {
public:
    explicit CriticalSectionIsr(portMUX_TYPE& mux);
    ~CriticalSectionIsr();

private:
    portMUX_TYPE& _mux;
};

} // namespace esp32pp
