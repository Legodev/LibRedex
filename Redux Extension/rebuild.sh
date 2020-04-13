rm -rvf build/* debug-build/* Binaries/*dll Binaries/*so Binaries/Debug/ Binaries/Release/ Binaries/Linux/  Binaries/LinuxAlt/ Binaries/Windows/
sh build-debug-all.sh
sh build-all.sh
cd Binaries/
7z a libredex-$(date +%Y%m%d%H%M).7z Linux/ Windows/ libredex.cfg COPYING ../Source/constants.hpp ../Source/
cd ..
