cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(try-llvm LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# llvm-config --cxxflags
add_compile_options(-fno-exceptions -fno-rtti)
include_directories(${LLVM_INCLUDE})
# llvm-config --ldflags
link_directories(${LLVM_LIB})

add_executable(try-llvm
        src/main.cpp
        src/lexer.cpp
        src/ast.cpp
)
# llvm-config --libs core
target_link_libraries(try-llvm LLVMCore LLVMRemarks LLVMBitstreamReader LLVMBinaryFormat LLVMSupport LLVMDemangle)
