# C++ standard required
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# checking compiler
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(CLANG 1)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(GCC 1)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
    set(INTEL 1)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(MSVC 1)
endif()

# setting flags
if(MSVC)
    add_definitions("/MP /W4") # /WX
    add_definitions("/std:c++latest")
    add_definitions("-DBGS_VISUAL_STUDIO")

    add_definitions("/wd4251") # warning with derived classes and dll interfaces (exporting stl) TODO: Remove after creating own containers
    add_definitions("/wd4201") # allow for unnamed struct/unions

    # disable exceptions
    add_definitions(-D_HAS_EXCEPTIONS=0)
    string(REGEX REPLACE "/EH[ascr-]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

endif()
