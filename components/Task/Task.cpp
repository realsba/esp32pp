// file   : Task.cpp
// author : sba <bohdan.sadovyak@gmail.com>

#include "Task.hpp"

#include <esp_log.h>

namespace esp32pp {

constexpr auto TAG = "Task";

Task::Task(Function func, std::string name, uint32_t stackSize, UBaseType_t priority, BaseType_t coreId)
    : _function(std::move(func))
    , _name(std::move(name))
{
    auto res = xTaskCreatePinnedToCore(&task_function, _name.c_str(), stackSize, this, priority, &_handle, coreId);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Can not create task \"%s\": %d", _name.c_str(), res);
    }
}

TaskHandle_t Task::getNativeHandle() const
{
    return _handle;
}

void Task::terminate()
{
    _running = false;
}

void Task::suspend()
{
    vTaskSuspend(_handle);
}

void Task::resumeFromISR()
{
    xTaskResumeFromISR(_handle);
}

void Task::task_function(void* instance)
{
    static_cast<Task*>(instance)->run();
}

void Task::run()
{
    ESP_LOGI(TAG, ">> %s", _name.c_str());

    _running = true;

    while (_running) {
        _function();
    }

    if (_handle != nullptr) {
        vTaskDelete(_handle);
        _handle = nullptr;
    }

    ESP_LOGI(TAG, "<< %s", _name.c_str());
}

} // namespace esp32pp
