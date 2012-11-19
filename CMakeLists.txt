#DEPENDENCIES
# - libcurl
# - pthreads
# - crypto++
# - gtest (not for the final release, but for debugging)
# TODO :: throw build errors if packages aren't found
# TODO :: search for each package

cmake_minimum_required (VERSION 2.6)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

# Build dependencies
# Build gtest libs
execute_process(WORKING_DIRECTORY ./third_party/googletest
                COMMAND cmake .
                RESULT_VARIABLE ret_var)

execute_process(WORKING_DIRECTORY ./third_party/googletest
                COMMAND make
                RESULT_VARIABLE ret_var)

execute_process(WORKING_DIRECTORY ./third_party/c5
               COMMAND make clean
               RESULT_VARIABLE ret_var)

execute_process(WORKING_DIRECTORY ./third_party/c5
               COMMAND make static)

execute_process(WORKING_DIRECTORY ./third_party/c5
                COMMAND mkdir lib
                RESULT_VARIABLE ret_var)

execute_process(WORKING_DIRECTORY ./third_party/c5
                COMMAND cp libcryptopp.a ./lib)
           
execute_process(WORKING_DIRECTORY ./third_party/c5
               COMMAND make clean
               RESULT_VARIABLE ret_var)

# Set additional include directories
INCLUDE_DIRECTORIES(./third_party/googletest/include)
INCLUDE_DIRECTORIES(./third_party/c5)
INCLUDE_DIRECTORIES(./third_party/sqlite)

INCLUDE_DIRECTORIES(./src/scrypt/lib/crypto)
INCLUDE_DIRECTORIES(./src/scrypt/lib/util)
INCLUDE_DIRECTORIES(./src/scrypt/lib/scryptenc)
INCLUDE_DIRECTORIES(./src/scrypt/lib)

# This is where binary files will go
file(MAKE_DIRECTORY ./bin)
set (EXECUTABLE_OUTPUT_PATH ./bin)

project(libattic)

file(GLOB_RECURSE libattic_SOURCE 
    "src/*.cpp"
    "src/*.c")

file(GLOB_RECURSE libattic_HEADERS 
    "src/*.h")

#file(GLOB_RECURSE libattic_HEADERS "src/*.h")

set(libattic_INCLUDE_DIRS "")

foreach (_headerFile ${libattic_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND libattic_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES libattic_INCLUDE_DIRS)

#Scrypt

#file(GLOB_RECURSE scrypt_SOURCE "third_party/scrypt/lib/*/*.c")
#file(GLOB_RECURSE scrypt_HEADERS "third_party/scrypt/lib/*/*.h")

#foreach (_headerFile ${scrypt_HEADERS})
#    get_filename_component(_dir ${_headerFile} PATH)
#    list (APPEND libattic_INCLUDE_DIRS ${_dir})
#endforeach()

#list(REMOVE_DUPLICATES libattic_INCLUDE_DIRS)






# Include our files
include_directories(${libattic_INCLUDE_DIRS})

# Locate GTest this is used if we install the lib 
# on the system iteslf, not necessary for multiplatform
# at the moment.
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})

# Add gtest lib to statically link
add_library(imp_gtest STATIC IMPORTED)
set_property(TARGET imp_gtest PROPERTY IMPORTED_LOCATION ./third_party/googletest/libgtest.a)

# Add crpytopp lib to statically link
add_library(imp_cryptpp STATIC IMPORTED)
set_property(TARGET imp_cryptpp PROPERTY IMPORTED_LOCATION ./third_party/c5/lib/libcryptopp.a)

#ADD_LIBRARY(scrypt STATIC ./src/sha256.c ./src/crypto_scrypt-nosse.c)
# build executable
add_executable(libattic ${libattic_SOURCE})

#link libs
#target_link_libraries(libattic curl ${GTEST_LIBRARIES} pthread)
target_link_libraries(libattic curl imp_gtest imp_cryptpp pthread)
