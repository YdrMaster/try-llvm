cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(try-llvm LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(try-llvm
        src/main.cpp
        src/token.cpp
        src/ast.cpp
)
