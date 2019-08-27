cmake_minimum_required(VERSION 3.2)

project(SimpleUpdateGUI)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find installed library
find_package(AppImageUpdaterBridge)

add_executable(SimpleUpdateGUI main.cc)

target_link_libraries(SimpleUpdateGUI PRIVATE AppImageUpdaterBridge)
