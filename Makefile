.PHONY : build build-debug clean

TYPE ?=Release

build:
	mkdir -p build/release
	cd build/release \
	&& cmake -DCMAKE_BUILD_TYPE=$(TYPE) ../.. \
	         -DLLVM_DIR=`llvm-config --libdir` \
	&& make -j2

clean:
	rm -rf build
