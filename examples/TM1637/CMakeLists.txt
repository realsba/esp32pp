cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTRA_COMPONENT_DIRS
        ../../TM1637
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(TM1637)
