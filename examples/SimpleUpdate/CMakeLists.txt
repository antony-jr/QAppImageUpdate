cmake_minimum_required(VERSION 3.2)

project(SimpleUpdate)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find installed library
find_package(AppImageUpdaterBridge)

add_executable(SimpleUpdate main.cc cutelog.c)

target_link_libraries(SimpleUpdate PRIVATE AppImageUpdaterBridge)
