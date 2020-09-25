CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(Update)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(QAppImageUpdate)

# Include Directories.
include_directories(.)
include_directories(${CMAKE_BINARY_DIR})

add_executable(Update main.cc)
target_link_libraries(Update PRIVATE QAppImageUpdate)
