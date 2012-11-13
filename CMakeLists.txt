#DEPENDENCIES
# - libcurl
# - pthreads
# - gtest (not for the final release, but for debugging)
# TODO :: throw build errors if packages aren't found
# TODO :: search for each package

cmake_minimum_required (VERSION 2.6)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

# TODO :: Set compiler flags

# This is where binary files will go
set (EXECUTABLE_OUTPUT_PATH ./bin)

project(libattic)

file(GLOB_RECURSE libattic_SOURCE "src/*.cpp")
file(GLOB_RECURSE libattic_HEADERS "src/*.h")

set(libattic_INCLUDE_DIRS "")
foreach (_headerFile ${libattic_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND libattic_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES libattic_INCLUDE_DIRS)
# Include our files
include_directories(${libattic_INCLUDE_DIRS})

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# build executable
add_executable(libattic ${libattic_SOURCE})

#link libs
target_link_libraries(libattic curl ${GTEST_LIBRARIES} pthread)
