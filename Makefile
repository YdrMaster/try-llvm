.PHONY : build run clean llvm internel_env

PROJ_DIR     := $(shell pwd)

LLVM_VERSION ?= 15.0.6
LLVM_TARGET  ?= clang+llvm-$(LLVM_VERSION)-x86_64-linux-gnu-ubuntu-18.04
LLVM_DIR     ?= $(PROJ_DIR)/llvm/$(LLVM_TARGET)
TYPE         ?= release

LLVM_INC     := $(LLVM_DIR)/include
LLVM_LIB     := $(LLVM_DIR)/lib

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
	wget -P llvm https://github.com/llvm/llvm-project/releases/download/llvmorg-$(LLVM_VERSION)/$(LLVM_TARGET).tar.xz
	tar xvf llvm/$(LLVM_TARGET).tar.xz -C llvm
