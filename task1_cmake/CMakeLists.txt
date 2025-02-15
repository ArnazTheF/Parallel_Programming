cmake_minimum_required(VERSION 3.10)
project(task1)

set(CMAKE_CXX_STANDARD 17)

option(FLOAT "Use float instead of double" OFF)

if(FLOAT)
    add_definitions(-DUSE_FLOAT)
    message(STATUS "Using float")
else()
    message(STATUS "Using double")
endif()

add_executable(${PROJECT_NAME} main.cpp)