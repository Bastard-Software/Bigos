cmake_minimum_required(VERSION 3.24)

project(DraftSandbox)
file(GLOB_RECURSE FILES *.h *.cpp)

include ("${ROOT_DIR}/CMakeScripts/CompilerSettings.cmake" NO_POLICY_SCOPE)
include ("${ROOT_DIR}/CMakeScripts/CompilerDefinitions.cmake" NO_POLICY_SCOPE)

add_executable(${PROJECT_NAME} ${FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${INCLUDE_DIR}/
) 

target_link_libraries(${PROJECT_NAME} PRIVATE
  Core
  Driver
  Platform
)

include("${SAMPLES_DIR}/CMakeScripts/SampleProperties.cmake" NO_POLICY_SCOPE)