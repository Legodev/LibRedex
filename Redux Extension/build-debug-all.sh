#!/usr/bin/env bash
mkdir -p build
cd build
####
mkdir -p 32bit-linux
cd 32bit-linux
cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Debug
make -j9
cd ..
####
mkdir -p 64bit-linux
cd 64bit-linux
cmake ../../ -DBITVALUE=64 -DCMAKE_BUILD_TYPE=Debug
make -j9
cd ..
####
mkdir -p 32bit-windows
cd 32bit-windows
/opt/mxe/usr/bin/i686-w64-mingw32.static.posix-cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Debug
make
cp -f libredex.dll ../../Binaries
cd ..
####
mkdir -p 64bit-windows
cd 64bit-windows
/opt/mxe/usr/bin/x86_64-w64-mingw32.static.posix-cmake ../../ -DBITVALUE=64 -DCMAKE_BUILD_TYPE=Debug
make
cp -f libredex_x64.dll ../../Binaries
cd ..
####
cd ..
