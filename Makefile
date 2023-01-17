.PHONY : build run clean llvm internel_env

PROJ_DIR := $(shell pwd)

LLVM_DIR ?= $(PROJ_DIR)/llvm/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz
TYPE     ?= release

LLVM_INC := $(LLVM_DIR)/include
LLVM_LIB := $(LLVM_DIR)/lib

build:
	mkdir -p build/$(TYPE)
	cd build/$(TYPE)                          \
	&& cmake -DCMAKE_BUILD_TYPE=$(TYPE) ../.. \
	         -DLLVM_INCLUDE=$(LLVM_INC)       \
	         -DLLVM_LIB=$(LLVM_LIB)           \
	&& make -j2

run: build
	@ echo
	@ $(PROJ_DIR)/build/$(TYPE)/try-llvm

clean:
	rm -rf build

llvm:
	mkdir -p llvm
	wget -P llvm https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.6/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	tar xvf llvm/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz -C llvm
