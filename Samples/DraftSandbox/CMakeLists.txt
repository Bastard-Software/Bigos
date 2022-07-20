cmake_minimum_required(VERSION 3.24)

project(DraftSandbox)
file(GLOB_RECURSE FILES *.h *.cpp)

add_executable(${PROJECT_NAME} ${FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${INCLUDE_DIR}/
) 

include("${SAMPLES_DIR}/CMakeScripts/SampleProperties.cmake" NO_POLICY_SCOPE)