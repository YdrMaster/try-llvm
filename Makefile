.PHONY : build run clean llvm internel_env
# 常量
PROJ_DIR     := $(shell pwd)
# 参数
LLVM_VERSION ?= 15.0.6
LLVM_TARGET  ?= clang+llvm-$(LLVM_VERSION)-x86_64-linux-gnu-ubuntu-18.04
LLVM_DIR     ?= $(PROJ_DIR)/llvm-$(LLVM_VERSION)/$(LLVM_TARGET)
TYPE         ?= release

build:
	mkdir -p build/$(TYPE)
	cd build/$(TYPE)                          \
	&& cmake -DCMAKE_BUILD_TYPE=$(TYPE) ../.. \
	         -DLLVM_DIR=$(LLVM_DIR)           \
	&& make -j2

run: build
	@ echo
	@ $(PROJ_DIR)/build/$(TYPE)/try-llvm

clean:
	rm -rf build

llvm:
	mkdir -p llvm-$(LLVM_VERSION)
	wget  -P llvm-$(LLVM_VERSION) https://github.com/llvm/llvm-project/releases/download/llvmorg-$(LLVM_VERSION)/$(LLVM_TARGET).tar.xz
	tar  xvf llvm-$(LLVM_VERSION)/$(LLVM_TARGET).tar.xz -C llvm-$(LLVM_VERSION)
