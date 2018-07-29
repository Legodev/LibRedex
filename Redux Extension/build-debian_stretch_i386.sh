#!/usr/bin/env bash
rm -rf Binaries/*so
mkdir -p build
cd build
####
mkdir -p 32bit-linux-debian-strech-release
cd 32bit-linux-debian-strech-release
cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Release
make
cd ..
####
cd ../Binaries
mkdir -p Linux_Debian_Stretch/Release/
mv libredex*.so Linux_Debian_Stretch/Release/
####
cd ../build
####
mkdir -p 32bit-linux-debian-strech-debug
cd 32bit-linux-debian-strech-debug
cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Debug
make
cd ..
####
cd ../Binaries
mkdir -p Linux_Debian_Stretch/Debug/
mv libredex*.so Linux_Debian_Stretch/Debug/
