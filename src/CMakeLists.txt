set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtCore library
find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5Concurrent)

include_directories(. ../QAIUpdateInformation ../libzsync ../librcksum ../libz)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(AIUpdaterBridge AIUpdaterBridge.cpp AIUpdaterBridge.hpp legacy_http.c)
target_link_libraries(AIUpdaterBridge  PUBLIC QAIUpdateInformation Qt5::Core Qt5::Concurrent Qt5::Network curl libzsync)
target_include_directories(AIUpdaterBridge PUBLIC . libzsync librcksum zlib QAIUpdateInformation)

