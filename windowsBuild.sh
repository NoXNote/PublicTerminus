#!/bin/bash
cmake -G "MinGW Makefiles" -B build -S . && mingw32-make -C build
cd build
mingw32-make Shaders 
#./rEngb.exe
cd ..