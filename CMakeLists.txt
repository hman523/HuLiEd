cmake_minimum_required(VERSION 3.10)
project(hulied)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Os -s")

add_executable(hulied src/main.cpp)
