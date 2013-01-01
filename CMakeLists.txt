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

set(CMAKE_CXX_FLAGS "-g")

# Build dependencies
# Build gtest libs
execute_process(WORKING_DIRECTORY ./third_party/jsoncpp
               COMMAND scons platform=linux-gcc)
               #RESULT_VARIABLE ret_var)

# TODO :: get this working, the json lib created the lib and folder 
# based on the version of gcc, could change per system
# for now change it by hand, later get it working dynamically
#set (${curdir} ./third_party/jsoncpp/libs)
#FILE(GLOB child RELATIVE ${curdir}/*)
#message(STATUS ${curdir})
#message(STATUS child)

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
               COMMAND make static
               RESULT_VARIABLE ret_var)

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
INCLUDE_DIRECTORIES(./third_party/jsoncpp/include)
INCLUDE_DIRECTORIES(./third_party/scryptlib)

# This is where binary files will go
file(MAKE_DIRECTORY ./bin)
set (EXECUTABLE_OUTPUT_PATH ./bin)

project(libattic)

file(GLOB_RECURSE libattic_SOURCE 
    "src/*.cpp"
    "src/*.c"
    "third_party/sqlite/*.c"
    "third_party/scryptlib/*.c")


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

# Add jsoncpp
add_library(imp_jsoncpp STATIC IMPORTED)
#set_property(TARGET imp_jsoncpp PROPERTY IMPORTED_LOCATION ./third_party/jsoncpp/libs/linux-gcc-4.7/libjson_linux-gcc-4.7_libmt.a)
set_property(TARGET imp_jsoncpp PROPERTY IMPORTED_LOCATION ./third_party/jsoncpp/libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.a)

#ADD_LIBRARY(scrypt STATIC ./src/sha256.c ./src/crypto_scrypt-nosse.c)


###############################################################################
# To compile a static lib
#add_library(attic STATIC ${libattic_SOURCE})

## this doesn't need to be set
#SET_TARGET_PROPERTIES(attic PROPERTIES
#                      STATIC_LIBRARY_FLAGS "./third_party/c5/lib/libcryptopp.a ./third_party/jsoncpp/libs/linux-gcc-4.6/libjson_linux-gcc-4.6_libmt.a")


###############################################################################
# To compile the test executable
add_executable(attic ${libattic_SOURCE})

#link libs
target_link_libraries(attic curl imp_gtest imp_cryptpp imp_jsoncpp pthread)




