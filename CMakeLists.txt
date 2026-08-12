cmake_minimum_required(VERSION 4.3)
project(cpp_lab)

set(CMAKE_CXX_STANDARD 26)

add_executable(cpp_lab main.cpp
        vec.h)

target_compile_options(cpp_lab PRIVATE -Wall -Wextra)
