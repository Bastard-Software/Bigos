cmake_minimum_required(VERSION 3.24)

project(Driver)

include ("${ROOT_DIR}/CMakeScripts/CompilerSettings.cmake" NO_POLICY_SCOPE)
include ("${ROOT_DIR}/CMakeScripts/CompilerDefinitions.cmake" NO_POLICY_SCOPE)

set(INC_DIRS
    "${INCLUDE_DIR}/${PROJECT_NAME}/*.h"
    "${INCLUDE_DIR}/${PROJECT_NAME}/*.inl"
)

set(SRC_DIRS
    "${SOURCE_DIR}/${PROJECT_NAME}/*.h"
    "${SOURCE_DIR}/${PROJECT_NAME}/*.inl"
    "${SOURCE_DIR}/${PROJECT_NAME}/*.cpp"
)

include ("${ROOT_DIR}/CMakeScripts/IncludeFiles.cmake" NO_POLICY_SCOPE)

message(STATUS "Generating project file for ${PROJECT_NAME} in ${PROJECT_SOURCE_DIR}")

add_library(${PROJECT_NAME} STATIC
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${THIRD_PARTY_DIR}/DXCompiler/bin/dxcompiler.dll
    ${OUTPUT_DIR}/dxcompiler.dll)

add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${THIRD_PARTY_DIR}/DXCompiler/bin/dxil.dll
    ${OUTPUT_DIR}/dxil.dll)

add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${THIRD_PARTY_DIR}/D3D12/build/native/bin/x64/D3D12Core.dll
    ${OUTPUT_DIR}/D3D12/D3D12Core.dll)

add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    ${THIRD_PARTY_DIR}/D3D12/build/native/bin/x64/d3d12SDKLayers.dll
    ${OUTPUT_DIR}/D3D12/d3d12SDKLayers.dll)

target_include_directories(Driver PRIVATE 
    ${THIRD_PARTY_DIR}/vulkan/include
    ${THIRD_PARTY_DIR}/volk
    ${THIRD_PARTY_DIR}/DXCompiler/include
    ${THIRD_PARTY_DIR}/D3D12/build/native/include
    ${THIRD_PARTY_DIR}/glm
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    d3d12
    dxgi
    dxguid
    volk
    setupapi
)

include ("${ROOT_DIR}/CMakeScripts/TargetProperties.cmake" NO_POLICY_SCOPE)