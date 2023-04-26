cmake_minimum_required(VERSION 3.24)

project(Bigos)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_DIR "${CMAKE_SOURCE_DIR}/CMakeScripts")
set(ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SAMPLES_DIR "${CMAKE_SOURCE_DIR}/Samples")
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/src/ThirdParty")

if(BGS_VULKAN_API)
    add_subdirectory(${THIRD_PARTY_DIR}/volk)
endif()
 
add_subdirectory(${SOURCE_DIR}/Core)
add_subdirectory(${SOURCE_DIR}/Driver)
add_subdirectory(${SOURCE_DIR}/Platform)

add_subdirectory(${SOURCE_DIR}/BigosEngine)

add_subdirectory(${SAMPLES_DIR}/DraftSandbox)



