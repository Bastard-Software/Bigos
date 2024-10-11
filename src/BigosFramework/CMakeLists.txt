cmake_minimum_required(VERSION 3.24)

project(BigosFramework)

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

include("${ROOT_DIR}/CMakeScripts/IncludeFiles.cmake" NO_POLICY_SCOPE)

add_library(${PROJECT_NAME} SHARED
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${INCLUDE_DIR}
    ${THIRD_PARTY_DIR}/glm/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Core
    Driver
    Platform
)

include ("${ROOT_DIR}/CMakeScripts/TargetProperties.cmake" NO_POLICY_SCOPE)