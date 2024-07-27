#!/bin/bash
gcc -O3 -std=c99 -W -Wall main.c pocketfft-master/pocketfft.c -lm -o linux
x86_64-w64-mingw32-gcc -O3 -g main.c pocketfft-master/pocketfft.c -lm -std=c99 -o windows.exe -D__USE_MINGW_ANSI_STDIO=0
