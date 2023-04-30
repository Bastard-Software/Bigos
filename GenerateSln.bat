@echo off

IF EXIST cmake_build RMDIR /S /Q cmake_build

rem Create build directory
mkdir cmake_build
cd cmake_build

rem Generate Visual Studio project files
cmake -G "Visual Studio 17 2022" ..

PAUSE