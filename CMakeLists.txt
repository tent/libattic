#DEPENDENCIES
# - libcurl
# - pthreads

cmake_minimum_required (VERSION 2.8)

option (ATTIC_STATIC_LIB "Build Attic Static lib" ON)
option (ATTIC_BINARY "Build Attic Binary test executable" OFF)

# Set compilers to use
set (CMAKE_CXX_COMPILER clang++)
set (CMAKE_C_COMPILER clang)

set(CMAKE_CXX_FLAGS "-g")

# Build dependencies
# Build gtest libs

project(libattic)
add_subdirectory("vendor/jsoncpp")
add_subdirectory("vendor/cryptopp")
#add_subdirectory("vendor/cpp-netlib-0.9.4")

if(ATTIC_BINARY)
    message(STATUS " including googletest includes")
    add_subdirectory("vendor/googletest")
endif(ATTIC_BINARY)

# Set additional include directories
if(ATTIC_BINARY)
    message(STATUS " including googletest includes")
    INCLUDE_DIRECTORIES(vendor/googletest/include)
endif(ATTIC_BINARY)

INCLUDE_DIRECTORIES(vendor/cryptopp)
INCLUDE_DIRECTORIES(vendor/jsoncpp/include)
INCLUDE_DIRECTORIES(vendor/sqlite)
INCLUDE_DIRECTORIES(vendor/scrypt)
INCLUDE_DIRECTORIES(vendor/base64)
INCLUDE_DIRECTORIES(vendor/cpp-netlib-0.9.4)
INCLUDE_DIRECTORIES(vendor/boost_1_53_0)

if(ATTIC_BINARY)
    # This is where binary files will go
    file(MAKE_DIRECTORY bin)
    set (EXECUTABLE_OUTPUT_PATH bin)
endif(ATTIC_BINARY)

if(ATTIC_STATIC_LIB)
    file(MAKE_DIRECTORY lib)
    set(LIBRARY_OUTPUT_PATH lib)
endif(ATTIC_STATIC_LIB)


file(GLOB_RECURSE libattic_SOURCE
    "src/*.cpp"
    "src/*.c"
    "vendor/sqlite/*.c"
    "vendor/scrypt/*.c"
    "vendor/base64/*.cpp")

if(ATTIC_STATIC_LIB)
    file(GLOB MAIN_CPP "src/main.cpp")
    LIST(REMOVE_ITEM libattic_SOURCE ${MAIN_CPP})
endif(ATTIC_STATIC_LIB)

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
#add_library(imp_gtest STATIC IMPORTED)
#set_property(TARGET imp_gtest PROPERTY IMPORTED_LOCATION ${gtest_lib})

# Build boost libs
if(EXISTS ./vendor/boost_1_53_0/bin.v2/)
else()
    execute_process(WORKING_DIRECTORY ./vendor/boost_1_53_0
                    COMMAND ./bootstrap.sh
                    COMMAND ./b2 link=static
                    RESULT_VARIABLE ret_var)
endif()

# Add boost threading
file(GLOB_RECURSE libboost_thread "vendor/boost_1_53_0/bin.v2/*libboost_thread.a")
add_library(imp_boost_thread STATIC IMPORTED)
set_property(TARGET imp_boost_thread PROPERTY IMPORTED_LOCATION ${libboost_thread})

# Add boost system
file(GLOB_RECURSE libboost_system "vendor/boost_1_53_0/bin.v2/*libboost_system.a")
add_library(imp_boost_system STATIC IMPORTED)
set_property(TARGET imp_boost_system PROPERTY IMPORTED_LOCATION ${libboost_system})

# Build cpp-netlib
# Add cpp-netlib
if(EXISTS ./vendor/cpp-netlib-0.9.4/libs/network/src/libcppnetlib-uri.a)
else()
execute_process(WORKING_DIRECTORY ./vendor/cpp-netlib-0.9.4
                    COMMAND cmake .
                    COMMAND make
                    RESULT_VARIABLE ret_var)
message("IGNORE CPP-NETLIB ERRORS")
endif()

file(GLOB_RECURSE libcppnetlib-uri "/vendor/cpp-netlib-0.9.4/libs/network/src/libcppnetlib-uri.a")
add_library(imp_cppnet_uri STATIC IMPORTED)
set_property(TARGET imp_cppnet_uri PROPERTY IMPORTED_LOCATION ${libcppnetlib-uri})


file(GLOB_RECURSE libcppnetlib-connections "/vendor/cpp-netlib-0.9.4/libs/network/src/libcppnetlib-client-connections.a")
add_library(imp_cppnet_client_connections STATIC IMPORTED)
set_property(TARGET imp_cppnet_client_connections PROPERTY IMPORTED_LOCATION ${libcppnetlib-connections})

if(ATTIC_STATIC_LIB)
    add_library(attic ${libattic_SOURCE} $<TARGET_OBJECTS:cryptlib> $<TARGET_OBJECTS:json> )
    #link lib
    target_link_libraries( attic 
                           imp_boost_thread
                           imp_boost_system
                           imp_cppnet_uri
                           imp_cppnet_client_connections
                           curl 
                           pthread 
                           ssl)
endif(ATTIC_STATIC_LIB)

if(ATTIC_BINARY)
    add_executable(attic ${libattic_SOURCE} $<TARGET_OBJECTS:cryptlib> $<TARGET_OBJECTS:json>) 
    #link libs
    target_link_libraries( attic 
                           imp_boost_thread
                           imp_boost_system
                           imp_cppnet_uri
                           imp_cppnet_client_connections
                           curl 
                           pthread 
                           ssl
                           gtest) #pulls gtest from google subdirectory

endif(ATTIC_BINARY)

