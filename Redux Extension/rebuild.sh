rm -rvf build/* debug-build/* Binaries/*dll Binaries/*so Binaries/Debug/* Binaries/Release/*
sh build-debug-all.sh
sh build-all.sh
cd Binaries/
7z a libredex-$(date +%Y%m%d%H%M).7z Debug/ Release/ libredex.cfg COPYING
cd ..
