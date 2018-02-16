CMAKE_MINIMUM_REQUIRED( VERSION 3.2)
project(AIUpdaterBridge)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

include_directories(. QAIUpdateInformation src libzsync librcksum libz)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# include subdirectories in the right order
add_subdirectory(zlib)
add_subdirectory(librcksum)
add_subdirectory(libzsync)
add_subdirectory(QAIUpdateInformation)
add_subdirectory(src)

