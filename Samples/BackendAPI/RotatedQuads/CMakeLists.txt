cmake_minimum_required(VERSION 3.24)

project(RotatedQuads)
file(GLOB_RECURSE FILES *.h *.cpp)

include ("${ROOT_DIR}/CMakeScripts/CompilerSettings.cmake" NO_POLICY_SCOPE)
include ("${ROOT_DIR}/CMakeScripts/CompilerDefinitions.cmake" NO_POLICY_SCOPE)

add_executable(${PROJECT_NAME} ${FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${INCLUDE_DIR}/
) 

target_link_libraries(${PROJECT_NAME} PRIVATE
  BIGOS
)

include("${SAMPLES_DIR}/BackendAPI/CMakeScripts/SampleProperties.cmake" NO_POLICY_SCOPE)