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

set(CMAKE_MODULE_PATH ${libattic_SOURCE_DIR}/cmake)

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

# Boost
include(ExternalProject)
ExternalProject_Add(boost
  URL "http://sourceforge.net/projects/boost/files/boost/1.53.0/boost_1_53_0.tar.bz2/download"
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh
  BUILD_COMMAND <SOURCE_DIR>/b2 install -j4 link=static --with-thread --with-system --with-date_time --with-filesystem --with-timer --with-chrono --prefix=<INSTALL_DIR>
  INSTALL_COMMAND ""
#--with-test --with-regex --with-date_time --with-filesystem --with-program_options 
)


ExternalProject_Get_Property(boost INSTALL_DIR)
set(BOOST_DIR ${INSTALL_DIR})
INCLUDE_DIRECTORIES(${BOOST_DIR}/include)


add_library(boost_thread STATIC IMPORTED)
set_property(TARGET boost_thread PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_thread.a)

add_library(boost_system STATIC IMPORTED)
set_property(TARGET boost_system PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_system.a)

add_library(boost_filesystem STATIC IMPORTED)
set_property(TARGET boost_filesystem PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_filesystem.a)

add_library(boost_datetime STATIC IMPORTED)
set_property(TARGET boost_datetime PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_date_time.a)

add_library(boost_timer STATIC IMPORTED)
set_property(TARGET boost_timer PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_timer.a)

add_library(boost_chrono STATIC IMPORTED)
set_property(TARGET boost_chrono PROPERTY IMPORTED_LOCATION ${BOOST_DIR}/lib/libboost_chrono.a)


if(ATTIC_STATIC_LIB)
    add_library(attic ${libattic_SOURCE} $<TARGET_OBJECTS:cryptlib> $<TARGET_OBJECTS:json> )
    #link lib
    target_link_libraries(attic
                          boost_thread
                          boost_system
                          boost_filesystem
                          boost_datetime
                          boost_timer
                          boost_chrono
                          pthread
                          ssl)


    include(mergestaticlibs)
    merge_static_libs(atticlib attic boost_thread boost_system boost_filesystem boost_datetime boost_timer boost_chrono)
endif(ATTIC_STATIC_LIB)

if(ATTIC_BINARY)
    add_executable(attic ${libattic_SOURCE} $<TARGET_OBJECTS:cryptlib> $<TARGET_OBJECTS:json>)
    #link libs
    target_link_libraries(attic
                          boost_thread
                          boost_system
                          boost_filesystem
                          boost_datetime
                          boost_timer
                          boost_chrono
                          pthread
                          ssl
                          rt
                          gtest) #pulls gtest from google subdirectory

endif(ATTIC_BINARY)

add_dependencies(attic boost)
