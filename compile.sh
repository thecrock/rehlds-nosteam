#!/bin/sh
#

mkdir bin 2>/dev/null
rm -f bin/dproto_i386.so 2>/dev/null

/opt/intel/bin/icc -mia32 -O3 -s -static-intel -shared -static-libgcc -no-intel-extensions \
	-Isrc \
	-Idep/hlsdk/common -Idep/hlsdk/dlls -Idep/hlsdk/engine -Idep/hlsdk/public \
	-Idep/metamod \
	src/*.cpp \
	-o bin/rehlds-nosteam.so

