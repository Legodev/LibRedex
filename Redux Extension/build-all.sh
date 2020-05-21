#!/usr/bin/env bash
mkdir -p build
cd build
####
mkdir -p 32bit-linux
cd 32bit-linux
cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-fPIC -m32" -DCMAKE_CXX_FLAGS="-fPIC -m32" -DFETCHCONTENT_SOURCE_DIR_BOOST="/home/lego/DesolationREDUX/Arma_3_Extensions/libredex/boost-src"
make -j 16
cd ..
####
mkdir -p 64bit-linux
cd 64bit-linux
cmake ../../ -DBITVALUE=64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC" -DFETCHCONTENT_SOURCE_DIR_BOOST="/home/lego/DesolationREDUX/Arma_3_Extensions/libredex/boost-src"
make -j 16
cd ..
####
mkdir -p 32bit-windows
cd 32bit-windows
/opt/mxe/usr/bin/i686-w64-mingw32.static.posix-cmake ../../ -DBITVALUE=32 -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_BOOST="/home/lego/DesolationREDUX/Arma_3_Extensions/libredex/boost-src"
make -j 16
cp -f libredex.dll ../../Binaries
cd ..
####
mkdir -p 64bit-windows
cd 64bit-windows
/opt/mxe/usr/bin/x86_64-w64-mingw32.static.posix-cmake ../../ -DBITVALUE=64 -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_BOOST="/home/lego/DesolationREDUX/Arma_3_Extensions/libredex/boost-src"
make -j 16
cp -f libredex_x64.dll ../../Binaries
cd ..
####
cd ../Binaries
mkdir -p Windows/Release/
mkdir -p Linux/Release/
mv libredex*.dll Windows/Release/
mv libredex*.so Linux/Release/
