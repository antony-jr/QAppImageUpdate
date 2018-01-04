CMAKE_MINIMUM_REQUIRED( VERSION 3.2)

project(AIUpdaterBridge)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtCore library
find_package(Qt5Core)

include_directories(. QAIUpdateInformation)
# config.h contains some global defines, our config.h.in is a modified version of the upstream config.h.in to make it
# integrate with CMake
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
# since there's headers in the build directory now, include that as well
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)

add_library(AIUpdaterBridge AIUpdaterBridge.cpp AIUpdaterBridge.hpp)
target_link_libraries(AIUpdaterBridge PRIVATE libzsync)
target_link_libraries(AIUpdaterBridge  PUBLIC Qt5::Core)
target_include_directories(AIUpdaterBridge PUBLIC . libzsync librcksum zlib)

