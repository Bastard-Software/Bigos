cmake_minimum_required(VERSION 3.24)

project(Bigos)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_DIR "${CMAKE_SOURCE_DIR}/CMakeScripts")
set(ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SAMPLES_DIR "${CMAKE_SOURCE_DIR}/Samples")
 
add_subdirectory(${SOURCE_DIR}/Core)
add_subdirectory(${SOURCE_DIR}/Containers)
add_subdirectory(${SOURCE_DIR}/Driver)
add_subdirectory(${SOURCE_DIR}/Platform)

add_subdirectory(${SOURCE_DIR}/BigosEngine)

add_subdirectory(${SAMPLES_DIR}/DraftSandbox)



