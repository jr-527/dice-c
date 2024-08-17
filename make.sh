#!/bin/bash
# make is too complicated lol
gcc -Os -std=c99 -W -Wall main.c pocketfft-master/pocketfft.c -lm -o dice-linux
# Use O2 to compile because Os triggers Windows defender
x86_64-w64-mingw32-gcc -O2 -g main.c pocketfft-master/pocketfft.c -lm -std=c99 -o dice-windows.exe -D__USE_MINGW_ANSI_STDIO=0
