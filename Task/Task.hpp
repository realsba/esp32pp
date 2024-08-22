#ifndef ESP32PP_TASK_HPP
#define ESP32PP_TASK_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <functional>
#include <string>

namespace esp32pp {

class Task
{
public:
  using Function = std::function<void()>;

  explicit Task(
    Function func, std::string name = "Task", uint32_t stackSize = 4096,
    UBaseType_t priority = 5, BaseType_t coreId = tskNO_AFFINITY
  );

  TaskHandle_t getNativeHandle() const;
  void terminate();

  void suspend();
  void resumeFromISR();

private:
  static void task_function(void* instance);

  void run();

  Function m_function;
  const std::string m_name;
  TaskHandle_t m_handle {nullptr};
  volatile bool m_running {false};
};

} // namespace esp32pp

#endif // ESP32PP_TASK_HPP
