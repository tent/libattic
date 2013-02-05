#DEPENDENCIES
# - libcurl
# - pthreads
# - crypto++
# - gtest (not for the final release, but for debugging)
# TODO :: throw build errors if packages aren't found
# TODO :: search for each package

cmake_minimum_required (VERSION 2.8)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

set(CMAKE_CXX_FLAGS "-g -Wall -v")

# Build dependencies
# Build gtest libs

message(STATUS "******************** BUILDING JSONCPP ********************" )
file(GLOB_RECURSE jsoncpp_lib "./third_party/jsoncpp/libs/*.a")
if(jsoncpp_lib)
    message(STATUS "found jsoncpp :  [${jsoncpp_lib}]")
else()
    execute_process(WORKING_DIRECTORY ./third_party/jsoncpp
                   COMMAND scons platform=linux-gcc)
                   #RESULT_VARIABLE ret_var)
    file(GLOB_RECURSE jsoncpp_lib "./third_party/jsoncpp/libs/*.a")
endif(jsoncpp_lib)

message(STATUS "******************** BUILDING GOOGLE TEST ********************" )
file(GLOB_RECURSE gtest_lib "./third_party/googletest/*.a")
if(gtest_lib)
    message(STATUS "found Google test lib [${gtest_lib}].")
else()
    execute_process(WORKING_DIRECTORY ./third_party/googletest
                    COMMAND cmake .
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./third_party/googletest
                    COMMAND make
                    RESULT_VARIABLE ret_var)

    execute_process(WORKING_DIRECTORY ./third_party/googletest
                    COMMAND rm libgtest_main.a 
                    RESULT_VARIABLE ret_var)

    file(GLOB_RECURSE gtest_lib "./third_party/googletest/*.a")
endif(gtest_lib)

message(STATUS " FOUND GOOGLE TEST LIB : [${gtest_lib}].")

message(STATUS "******************** BUILDING CRYPTO ********************")
file(GLOB_RECURSE crypto_lib "./third_party/c5/lib/*.a")
if(crypto_lib)
    message(STATUS "found Cryptolib [${crypto_lib}].")
else()
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
    file(GLOB_RECURSE crypto_lib "./third_party/c5/lib/*.a")
endif(crypto_lib)

message(STATUS "FOUND Cryptolib [${crypto_lib}].")

# Set additional include directories
INCLUDE_DIRECTORIES(./third_party/googletest/include)
INCLUDE_DIRECTORIES(./third_party/c5)
INCLUDE_DIRECTORIES(./third_party/sqlite)
INCLUDE_DIRECTORIES(./third_party/jsoncpp/include)
INCLUDE_DIRECTORIES(./third_party/scryptlib)
INCLUDE_DIRECTORIES(./third_party/base64)

# This is where binary files will go
file(MAKE_DIRECTORY ./bin)
set (EXECUTABLE_OUTPUT_PATH ./bin)

file(MAKE_DIRECTORY ./lib)
set(LIBRARY_OUTPUT_PATH ./lib)

project(libattic)

file(GLOB_RECURSE libattic_SOURCE 
    "src/*.cpp"
    "src/*.c"
    "third_party/sqlite/*.c"
    "third_party/scryptlib/*.c"
    "third_party/base64/*.cpp")

file(GLOB_RECURSE libattic_HEADERS 
    "src/*.h")
#file(GLOB_RECURSE libattic_HEADERS "src/*.h")

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

# Add crpytopp lib to statically link
add_library(imp_cryptpp STATIC IMPORTED)
set_property(TARGET imp_cryptpp PROPERTY IMPORTED_LOCATION ${crypto_lib})

# Add jsoncpp
add_library(imp_jsoncpp STATIC IMPORTED)
set_property(TARGET imp_jsoncpp PROPERTY IMPORTED_LOCATION ${jsoncpp_lib})

###############################################################################
# To compile a static lib
add_library(attic STATIC ${libattic_SOURCE})

# copy libs to lib folder
execute_process(COMMAND cp ${crypto_lib} ./lib)
execute_process(COMMAND cp ${jsoncpp_lib} ./lib)
 
# archiver commands -- commands
file(GLOB libs "./lib/*.a")
foreach(lib ${libs})
    message(STATUS "EXTRACTING : [${lib}]" )
    execute_process(WORKING_DIRECTORY ./lib
                    COMMAND ar -x ${lib}
                    RESULT_VARIABLE ret_var)
endforeach()
foreach(lib ${libs})
    message(STATUS "DELETING : [${lib}]" )
    execute_process(WORKING_DIRECTORY ./lib
                    COMMAND rm ${lib}
                    RESULT_VARIABLE ret_var)
endforeach()

file(GLOB objects "./lib/*.o")
foreach(obj ${objects})
    execute_process(WORKING_DIRECTORY ./lib
                    COMMAND ar -rv libattic.a ${obj}
                    RESULT_VARIABLE ret_var)
    execute_process(WORKING_DIRECTORY ./lib
                    COMMAND rm ${obj}
                    RESULT_VARIABLE ret_var)
endforeach()

###############################################################################
# To compile the test executable
#add_executable(attic ${libattic_SOURCE})

#link libs
#target_link_libraries(attic curl imp_gtest imp_cryptpp imp_jsoncpp pthread)




