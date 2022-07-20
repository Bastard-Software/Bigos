cmake_minimum_required(VERSION 3.24)

file(GLOB_RECURSE INCLUDE_FILES ${INC_DIRS})
file(GLOB_RECURSE SOURCE_FILES ${SRC_DIRS})

# takes a paths of all files included in project,
# extract folder names between "include" folder and filename for each path 
# then adding it to proper source_group
foreach(f ${INCLUDE_FILES})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH GRP ${INCLUDE_DIR} ${f})
    set(GRP "include/${GRP}")
    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" GRP ${GRP})
    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ GRP ${GRP})
    source_group("${GRP}" FILES ${f})
    # message(STATUS "${GRP}")
endforeach()

# takes a paths of all files source in project,
# extract folder names between "src" folder and filename for each path 
# then adding it to proper source_group
foreach(f ${SOURCE_FILES})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH GRP ${SOURCE_DIR} ${f})
    set(GRP "src/${GRP}")
    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" GRP ${GRP})
    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ GRP ${GRP})
    source_group("${GRP}" FILES ${f})
    # message(STATUS "${GRP}")
endforeach()

include_directories("${INCLUDE_DIR}" "${SOURCE_DIR}")