#!/bin/bash
# make is too complicated lol
# build for linux. I use Os because from testing on my computer it seems to be
# the fastest.
gcc -Os -std=c99 -c pocketfft/pocketfft.c -o pocketfft.o
gcc -Os -W -Wall -Wextra -Werror -std=c99 -c main.c -lm -o main.o
g++ -Os -Wall -Wextra -Werror -std=c++11 -c drop.cpp -o drop.o
g++ -Os -o dice-linux main.o drop.o pocketfft.o -static

# build for windows. we use mingw because MSVC somehow still doesn't properly
# support C complex numbers. I use -O2 because from testing on my computer,
# windows defender thinks it's a virus with -O3 or -Os.
x86_64-w64-mingw32-gcc -O2 -std=c99 -c pocketfft/pocketfft.c -o pocketfft-win.o -D__USE_MINGW_ANSI_STDIO=0
x86_64-w64-mingw32-gcc -O2 -std=c99 -c -lm main.c -o main-win.o -D__USE_MINGW_ANSI_STDIO=0
x86_64-w64-mingw32-g++ -O2 -std=c++11 -c drop.cpp -o drop-win.o -D__USE_MINGW_ANSI_STDIO=0
x86_64-w64-mingw32-g++ -O2 -o dice-windows.exe main-win.o drop-win.o pocketfft-win.o -static -D__USE_MINGW_ANSI_STDIO=0
