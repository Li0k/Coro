cmake_minimum_required(VERSION 3.10)
project(coro)

set(CMAKE_CXX_STANDARD 11)

add_executable(coro Coro.cpp Coro.h test1.cpp)