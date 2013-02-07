#DEPENDENCIES
# - libcurl
# - pthreads
# TODO :: throw build errors if packages aren't found
# TODO :: search for each package

cmake_minimum_required (VERSION 2.8)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

set(CMAKE_CXX_FLAGS "-g -Wall -v")

# Build dependencies
# Build gtest libs

project(libattic)
add_subdirectory("vendor/jsoncpp")
add_subdirectory("vendor/cryptopp")

message(STATUS "******************** BUILDING GOOGLE TEST ********************" )
file(GLOB_RECURSE gtest_lib "vendor/googletest/lib/*.a")
if(gtest_lib)
    message(STATUS "found Google test lib [${gtest_lib}].")
else()
    file(MAKE_DIRECTORY ./vendor/googletest/lib)

    execute_process(WORKING_DIRECTORY ./vendor/googletest
                    COMMAND cmake .
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./vendor/googletest
                    COMMAND make
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./vendor/googletest
                    COMMAND rm libgtest_main.a
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./vendor/googletest
                    COMMAND mv libgtest.a ./lib
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./vendor/googletest
                    COMMAND make clean
                    RESULT_VARIABLE ret_var)

    file(GLOB_RECURSE gtest_lib "vendor/googletest/*.a")
endif(gtest_lib)

message(STATUS " GOOGLE TEST LIB : [${gtest_lib}]")


# Set additional include directories
INCLUDE_DIRECTORIES(vendor/googletest/include)
INCLUDE_DIRECTORIES(vendor/cryptopp)
INCLUDE_DIRECTORIES(vendor/jsoncpp/include)
INCLUDE_DIRECTORIES(vendor/sqlite)
INCLUDE_DIRECTORIES(vendor/scrypt)
INCLUDE_DIRECTORIES(vendor/base64)

# This is where binary files will go
file(MAKE_DIRECTORY bin)
set (EXECUTABLE_OUTPUT_PATH bin)

file(MAKE_DIRECTORY lib)
set(LIBRARY_OUTPUT_PATH lib)

file(GLOB_RECURSE libattic_SOURCE
    "src/*.cpp"
    "src/*.c"
    "vendor/sqlite/*.c"
    "vendor/scrypt/*.c"
    "vendor/base64/*.cpp")

file(GLOB_RECURSE libattic_HEADERS
    "src/*.h")

set(libattic_INCLUDE_DIRS "")

foreach (_headerFile ${libattic_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND libattic_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES libattic_INCLUDE_DIRS)

# Include our files
include_directories(${libattic_INCLUDE_DIRS})

# Add gtest lib to statically link
add_library(imp_gtest STATIC IMPORTED)
set_property(TARGET imp_gtest PROPERTY IMPORTED_LOCATION ${gtest_lib})

add_library(attic STATIC ${libattic_SOURCE})

#link libs
target_link_libraries(attic curl imp_gtest cryptlib json pthread)
