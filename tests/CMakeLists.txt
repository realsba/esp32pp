cmake_minimum_required(VERSION 3.25)
project(UnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Catch2 3 REQUIRED)

include_directories(
    ${Catch2_INCLUDE_DIRS}
    ../managed_components/espressif__asio/asio/asio/include
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(tests
    ${TEST_SOURCES}
    ../LedEffects/ActionSequence.cpp
)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(tests)
