# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html

cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTRA_COMPONENT_DIRS
    $ENV{IDF_PATH}/components
    Button
    CriticalSection
    DHT
    HttpServer
    I2C
    Ina226
    LedEffects
    LedStrip
    PwmController
    RF433
    TM1637
    Task
    TelegramBot
    Timer
    WiFiStation
)

add_compile_definitions(ASIO_NO_DEPRECATED)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(esp32pp)
