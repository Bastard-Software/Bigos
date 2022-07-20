@echo off

rem Create build directory
mkdir cmake_build
cd cmake_build

rem Generate Visual Studio project files
cmake -G "Visual Studio 17 2022" ..

PAUSE