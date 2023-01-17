cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

project(try-llvm LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# llvm-config --cxxflags
add_compile_options(-fno-exceptions -fno-rtti)

EXECUTE_PROCESS(COMMAND ${LLVM_DIR}/bin/llvm-config --includedir
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE llvm_include)
EXECUTE_PROCESS(COMMAND ${LLVM_DIR}/bin/llvm-config --libdir
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE llvm_lib)
EXECUTE_PROCESS(COMMAND ${LLVM_DIR}/bin/llvm-config --libs core orcjit
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE llvm_link)

include_directories(${llvm_include})
link_directories(${llvm_lib})

add_executable(try-llvm
        src/main.cpp
        src/lexer.h
        src/lexer.cpp
        src/ast.h
        src/ast.cpp
        src/codegen.cpp
)
# llvm-config --libs ...
target_link_libraries(try-llvm ${llvm_link})
