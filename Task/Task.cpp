#include "Task.hpp"

#include <esp_log.h>

constexpr auto TAG = "Task";

Task::Task(Task::Function func, std::string name, uint32_t stackSize, UBaseType_t priority, BaseType_t coreId)
  : m_function(std::move(func))
  , m_name(std::move(name))
{
  auto res = xTaskCreatePinnedToCore(&task_function, m_name.c_str(), stackSize, this, priority, &m_handle, coreId);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Can not create task \"%s\": %d", m_name.c_str(), res);
  }
}

TaskHandle_t Task::getNativeHandle() const
{
  return m_handle;
}

void Task::terminate()
{
  m_running = false;
}

void Task::suspend()
{
  vTaskSuspend(m_handle);
}

void Task::resumeFromISR()
{
  xTaskResumeFromISR(m_handle);
}

void Task::task_function(void* instance)
{
  static_cast<Task*>(instance)->run();
}

void Task::run()
{
  ESP_LOGI(TAG, ">> %s", m_name.c_str());

  m_running = true;

  while (m_running) {
    m_function();
  }

  if (m_handle != nullptr) {
    vTaskDelete(m_handle);
    m_handle = nullptr;
  }

  ESP_LOGI(TAG, "<< %s", m_name.c_str());
}
