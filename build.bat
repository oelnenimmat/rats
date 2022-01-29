@echo off

clang++ ^
src/*.cpp prebuild.o ^
-o game.exe ^
-g -gcodeview ^
-std=c++17 ^
-fopenmp ^
-O0 ^
-D_CRT_SECURE_NO_WARNINGS ^
-Werror ^
-Iinclude ^
-I%VULKAN_SDK%/Include ^
-lUser32 -lwinmm ^
-l%VULKAN_SDK%/Lib/vulkan-1

:: -include-pch src/precompiled.pch ^
:: Not sure about _CRT_SECURE_NO_WARNINGS being here, it is not needed everywhere,
:: but i did not find where it was needed fast enough :(