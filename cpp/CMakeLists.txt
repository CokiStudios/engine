cmake_minimum_required(VERSION 3.14)
project(LoopingEngine VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8 /EHsc /O2)
else()
    add_compile_options(-O3 -Wall -Wextra -pthread)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(looping main.cpp)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Makefile")
    set(LOOPING_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
else()
    set(LOOPING_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")
endif()

set_target_properties(looping PROPERTIES
    OUTPUT_NAME "looping"
    RUNTIME_OUTPUT_DIRECTORY "${LOOPING_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${LOOPING_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${LOOPING_BIN_DIR}"
)
