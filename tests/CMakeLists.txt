CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(QAppImageUpdateTests VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT BUILD_TESTS)
	find_package(QAppImageUpdate)
endif()
find_package(Qt5Test)
find_package(Qt5Concurrent)

# Include Directories.
include_directories(.)
include_directories(${CMAKE_BINARY_DIR})

if(QUICK_TEST)
	add_definitions(-DQUICK_TEST)
endif()

if(DECENTRALIZED_UPDATE_ENABLED)
    message("-- [*] IMPORTANT: Decentralized update feature will be TESTED.")
    add_definitions(-DDECENTRALIZED_UPDATE_ENABLED)
endif()

add_executable(QAppImageUpdateTests main.cc QAppImageUpdateTests.hpp SimpleDownload.hpp)
target_link_libraries(QAppImageUpdateTests PRIVATE QAppImageUpdate Qt5::Test Qt5::Concurrent)
