#ifndef ESP32PP_CRITICALSECTION_HPP
#define ESP32PP_CRITICALSECTION_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

class CriticalSection
{
public:
  explicit CriticalSection(portMUX_TYPE& mux);
  ~CriticalSection();

private:
  portMUX_TYPE& m_mux;
};

class CriticalSectionIsr
{
public:
  explicit CriticalSectionIsr(portMUX_TYPE& mux);
  ~CriticalSectionIsr();

private:
  portMUX_TYPE& m_mux;
};

#endif // ESP32PP_CRITICALSECTION_HPP
