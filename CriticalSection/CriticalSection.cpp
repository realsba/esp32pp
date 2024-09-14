#include "CriticalSection.hpp"

#include <freertos/task.h>

namespace esp32pp {

CriticalSection::CriticalSection(portMUX_TYPE& mux)
    : m_mux(mux)
{
    taskENTER_CRITICAL(&m_mux);
}

CriticalSection::~CriticalSection()
{
    taskEXIT_CRITICAL(&m_mux);
}

CriticalSectionIsr::CriticalSectionIsr(portMUX_TYPE& mux)
    : m_mux(mux)
{
    taskENTER_CRITICAL_ISR(&m_mux);
}

CriticalSectionIsr::~CriticalSectionIsr()
{
    taskEXIT_CRITICAL_ISR(&m_mux);
}

} // namespace esp32pp
