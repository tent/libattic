cmake_minimum_required (VERSION 2.6)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

# TODO :: Set compiler flags

# This is where binary files will go
set (EXECUTABLE_OUTPUT_PATH ./bin)

project(test)


file(GLOB_RECURSE test_SOURCE "src/*.cpp")
file(GLOB_RECURSE test_HEADERS "src/*.h")

set(test_INCLUDE_DIRS "")
foreach (_headerFile ${test_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND test_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES test_INCLUDE_DIRS)

include_directories(${test_INCLUDE_DIRS})
# build executable
add_executable(test ${test_SOURCE})

#link libs
target_link_libraries(test curl)
