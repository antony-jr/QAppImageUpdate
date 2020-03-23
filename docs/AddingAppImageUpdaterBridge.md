---
id: AddingAppImageUpdaterBridge
title: Add AppImage Updater Bridge to your Project
sidebar_label: Add AppImage Updater Bridge to your Project.
---

AppImage Updater Bridge can compiled with both *QMake* and *CMake* , I recommend you to use *QMake* if you are
building your application statically with Qt. Use *CMake* for normal compilation and other stuff , This is because *CMake* is 
easy to use than *QMake* when compiling with shared libraries of Qt. But *CMake* is very hard to work with when using a static 
version of Qt , So to save some madness be a good boy/girl and use *QMake* instead.


# Using QMake.

## The Simple Way for Single Application.

* Go to your project folder.
* Install AppImageUpdaterBridge as a git submodule or just clone it.

* Add this line to your project file.
```
include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri)
```

* Add AppImageUpdaterBridge headers to your source file.
```
#include <AppImageUpdaterBridge>
```

* Finally , Just Compile.
```
 $ mkdir build ; cd build ; qmake .. ; make -j$(nproc) 
```

## The Hard And Standard Way for Large Applications and Libraries.

Since your project uses qmake as the makefile generator then you have to follow a specific directory 
hierarchy , The structure is show below.

```
 -MyCoolApplication
  --libs
     ---AppImageUpdaterBridge
     ---libs.pro
  --src
     ---main.cpp
     ---mainwindow.hpp
     ---mainwindow.cpp
     ---src.pro
  --MyCoolApplication.pro
```


### The Library Subdir Project file (libs.pro)

This is where you keep all third party libraries including **AppImageUpdaterBridge**.
Just add a **git submodule** or execute the steps mentioned in the **Installation**   
in the **libs** directory of your project folder.



```
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = AppImageUpdaterBridge
```

### Your Source's Project file (src.pro)

```
TEMPLATE = app
TARGET = ../MyCoolApplication

QT += core gui # Modules thats needed by your app.

# Put your dynamic libs after AppImageUpdaterBridge. 
LIBS += ../libs/AppImageUpdaterBridge/libAppImageUpdaterBridge.a 

INCLUDEPATH += . .. \
               ../libs/AppImageUpdaterBridge/ # This is important

SOURCES += main.cpp mainwindow.cpp # All your source files.
HEADERS += mainwindow.hpp # All your header files.
```

### Your Main Project file ( MyCoolApplication.pro )

```
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = libs \ # Always use this order
	  src
```


### Including AppImageUpdaterBridge in your Source

Whenever you want to use **AppImageUpdaterBridge** , you just need to include it!

```
#include <AppImageUpdaterBridge>
```


Thats it , All you have to do is to build your project with **qmake**.   

```
 $ mkdir build; cd build ; qmake .. ; make -j$(nproc) 
```


# Using CMake 

It is recommended for you to keep your source files at the top level and then add
AppImageUpdaterBridge as a subdirectory in your CMakeList.txt

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
find_package(Qt5Network)
find_package(Qt5Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR}) # just in case!
# Add include directory
include_directories(AppImageUpdaterBridge)
include_directories(AppImageUpdaterBridge/include)

# include subdirectories 
add_subdirectory(AppImageUpdaterBridge)

add_executable(MyCoolApplication MyMain.cpp)
target_link_libraries(MyCoolApplication AppImageUpdaterBridge 
					Qt5::Core 
					Qt5::Network
					Qt5::Widgets)
```

Where **MyCoolApplication** is the application that you are currently working with.

# Disable Logging in QMake or CMake

For some reasone if you think you don't want logging support whatsoever , i.e no signal from logger signal in
AppImageDeltaRevisioner. 

In QMake like this , 

```
 $ qmake "CONFIG+=LOGGING_DISABLED" [ProjectFolder]
```

and

In CMake like this ,

```
 $ cmake -DLOGGING_DISABLED=ON [ProjectFolder]
```

Compiling without logger support can reduce your binary by approx. 100 KiB and 
also saves some runtime overhead and memory usage.

# Disable building AppImageUpdaterDialog

AppImageUpdaterDialog is a class provided by the library for an easy use of the updater 
through a nice graphical modal dialog which can be handy at times.
But if you want to disable this for some reason then you can do so by doing like this ,

In QMake ,

```
 $ qmake "CONFIG+=NO_GUI" [ProjectFolder]
```

and 

In CMake ,

```
 $ cmake -DNO_GUI=ON [ProjectFolder]
```
