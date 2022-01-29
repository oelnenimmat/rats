@echo off

clang++ ^
src/prebuild/prebuild.cpp ^
-c ^
-o prebuild.o ^
-g -gcodeview ^
-std=c++17 ^
-O0 ^
-D_CRT_SECURE_NO_WARNINGS ^
-Werror ^
-Iinclude ^
-I%VULKAN_SDK%/Include

:: Not sure about _CRT_SECURE_NO_WARNINGS being here, it is not needed everywhere,
:: but i did not find where it was needed fast enough :(