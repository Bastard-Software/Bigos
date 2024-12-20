cmake_minimum_required(VERSION 3.24)

project(Platform)

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

target_include_directories(Platform PRIVATE 
    ${THIRD_PARTY_DIR}/glm
)

include ("${ROOT_DIR}/CMakeScripts/TargetProperties.cmake" NO_POLICY_SCOPE)