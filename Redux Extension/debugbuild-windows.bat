SET OLDPATH=%PATH%
if not exist 32bit mkdir 32bit
cd 32bit
set PATH=C:\MinGW\32bit\bin;%OLDPATH%
cmake -D BITVALUE=32 -D CMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..\..
C:\MinGW\64bit\bin\mingw32-make VERBOSE=1
cd ..
if not exist 64bit mkdir 64bit
cd 64bit
set PATH=C:\MinGW\64bit\bin;%OLDPATH%
cmake -D BITVALUE=64 -D CMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..\..
C:\MinGW\64bit\bin\mingw32-make VERBOSE=1
cd ..
set PATH=%OLDPATH%
