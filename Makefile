.PHONY : build run clean llvm

TYPE ?=release
PROJ_DIR = $(shell pwd)

build:
	mkdir -p build/release
	cd build/release \
	&& cmake -DCMAKE_BUILD_TYPE=$(TYPE) ../.. \
	         -DLLVM_INCLUDE=`llvm-config --includedir` \
	         -DLLVM_LIB=`llvm-config --libdir` \
	&& make -j2

run: build
	@ echo
	@ $(PROJ_DIR)/build/$(TYPE)/try-llvm

clean:
	rm -rf build

llvm:
	mkdir -p llvm
	wget -P llvm https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.6/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	tar xvf -C llvm llvm/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04.tar.xz
	export LLVM_DIR=$(PROJ_DIR)/llvm/clang+llvm-15.0.6-x86_64-linux-gnu-ubuntu-18.04
	export PATH=$PATH:$LLVM_DIR/bin
