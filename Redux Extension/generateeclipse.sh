#!/usr/bin/env bash
mkdir -p "../eclipse Redux Extension/"
cd  "../eclipse Redux Extension/"
cmake -G"Eclipse CDT4 - Unix Makefiles" -DBITVALUE=64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_VERSION=4.6 -DCMAKE_CXX_COMPILER_ARG1=-std=c++11 "../Redux Extension/"
