cmake_minimum_required(VERSION 3.25)
project(UnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 3 REQUIRED)

include(CTest)
include(Catch)
enable_testing()

include_directories(
    ${Catch2_INCLUDE_DIRS}
    ../managed_components/espressif__asio/asio/asio/include
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME}
    ${TEST_SOURCES}
    ../LedEffects/ActionSequence.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Catch2::Catch2WithMain
)

catch_discover_tests(${PROJECT_NAME})
