@echo off

::https://stackoverflow.com/questions/55885920/how-do-i-generate-and-use-precompiled-headers-with-clang
clang++ src/precompiled.hpp -o src/precompiled.pch -Iinclude -std=c++17 -fopenmp

:: https://stackoverflow.com/questions/26755219/how-to-use-pch-with-clang
::clang++ -x c++-header src/precompiled.hpp -emit-pch -o src/precompiled.pch