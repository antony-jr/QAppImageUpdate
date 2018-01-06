---
id: AddingAppImageUpdaterBridge
title: Add AppImage Updater Bridge to your Project
sidebar_label: Add AppImage Updater Bridge to CMake
---

|	        |	    	                                           |		
|-----------|------------------------------------------------------|
|  Header:  | #include "AppImageUpdaterBridge/AIUpdaterBridge.hpp" |
|   qmake:  | QT += core network		                           |
|	        | HEADERS += AppImageUpdaterBridge/AIUpdaterBridge.hpp |
|Inherits:  | [QThread](http://doc.qt.io/qt-5/qthread.html)        |


The recommend way to add **AppImage Updater Bridge** to your project is to use it as a **CMake** Subdirectory. It is recommended for you to keep your source files at the top level and then add   
**AppImageUpdaterBridge** as a **subdirectory** in your **CMakeList.txt**

Here is a small example on how it is done!

```
CMAKE_MINIMUM_REQUIRED( VERSION 3.2)
project(MyCoolApplication)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtCore library
find_package(Qt5Core)
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # just in case!
# Add include directory
include_directories(AppImageUpdaterBridge)

# include subdirectories 
add_subdirectory(AppImageUpdaterBridge)

add_executable(MyCoolApplication MyMain.cpp)
target_link_libraries(MyCoolApplication AIUpdaterBridge Qt5::Core)
```

Where **MyCoolApplication** is application that you are currently working with!
