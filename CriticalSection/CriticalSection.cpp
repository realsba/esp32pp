#include "CriticalSection.hpp"

#include <freertos/task.h>

namespace esp32pp {

CriticalSection::CriticalSection(portMUX_TYPE& mux)
    : _mux(mux)
{
    taskENTER_CRITICAL(&_mux);
}

CriticalSection::~CriticalSection()
{
    taskEXIT_CRITICAL(&_mux);
}

CriticalSectionIsr::CriticalSectionIsr(portMUX_TYPE& mux)
    : _mux(mux)
{
    taskENTER_CRITICAL_ISR(&_mux);
}

CriticalSectionIsr::~CriticalSectionIsr()
{
    taskEXIT_CRITICAL_ISR(&_mux);
}

} // namespace esp32pp
