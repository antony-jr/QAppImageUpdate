cmake_minimum_required(VERSION 3.2)

project(ProxyUpdate)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find installed library
find_package(AppImageUpdaterBridge)

add_executable(ProxyUpdate main.cc)

target_link_libraries(ProxyUpdate PRIVATE AppImageUpdaterBridge)
