cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(try-llvm LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(src)
link_directories(${LLVM_DIR})

add_executable(try-llvm
        src/main.cpp
        src/lexer.cpp
        src/ast.cpp
)
target_link_libraries(try-llvm LLVMCore LLVMRemarks LLVMBitstreamReader LLVMBinaryFormat LLVMSupport LLVMDemangle)
