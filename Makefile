.PHONY : build run clean

TYPE ?=release

build:
	mkdir -p build/release
	cd build/release \
	&& cmake -DCMAKE_BUILD_TYPE=$(TYPE) ../.. \
	         -DLLVM_INCLUDE=`llvm-config --includedir` \
	         -DLLVM_LIB=`llvm-config --libdir` \
	&& make -j2

run: build
	@ echo
	@ $(shell pwd)/build/$(TYPE)/try-llvm

clean:
	rm -rf build
