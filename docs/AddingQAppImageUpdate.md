---
id: AddingQAppImageUpdate
title: Add QAppImageUpdate to your Project
sidebar_label: Adding to your Project
---

QAppImage can compiled with both *QMake* and *CMake* , I recommend you to use *QMake* if you are
building your application statically with Qt. Use *CMake* for normal compilation and other stuff, This is because *CMake* is 
easy to use than *QMake* when compiling with shared libraries of Qt. But *CMake* is very hard to work with when using a static 
version of Qt , So to save some madness be a good boy/girl and use *QMake* instead.


# Using QMake.

## The Simple Way for Single Application.

* Go to your project folder.
* Install QAppImageUpdate as a git submodule or just clone it.

* Add this line to your project file.
```
include(QAppImageUpdate/QAppImageUpdate.pri)
```

* Add QAppImageUpdate headers to your source file.
```
#include <QAppImageUpdate>
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
     ---QAppImageUpdate
     ---libs.pro
  --src
     ---main.cpp
     ---mainwindow.hpp
     ---mainwindow.cpp
     ---src.pro
  --MyCoolApplication.pro
```


### The Library Subdir Project file (libs.pro)

This is where you keep all third party libraries including **QAppImageUpdate**.
Just add a **git submodule** or execute the steps mentioned in the **Installation**   
in the **libs** directory of your project folder.



```
TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = QAppImageUpdate
```

### Your Source's Project file (src.pro)

```
TEMPLATE = app
TARGET = ../MyCoolApplication

QT += core gui # Modules thats needed by your app.

# Put your dynamic libs after QAppImageUpdate. 
LIBS += ../libs/QAppImageUpdate/libQAppImageUpdate.a 

INCLUDEPATH += . .. \
               ../libs/QAppImageUpdate/ # This is important

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


### Including QAppImageUpdate in your Source

Whenever you want to use **QAppImageUpdate**, you just need to include it!

```
#include <QAppImageUpdate>
```


Thats it, All you have to do is to build your project with **qmake**.   

```
 $ mkdir build; cd build ; qmake .. ; make -j$(nproc) 
```

# Using CMake 

It is recommended for you to keep your source files at the top level and then add
QAppImageUpdate as a subdirectory in your CMakeList.txt

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

# include subdirectories 
add_subdirectory(QAppImageUpdate)

add_executable(MyCoolApplication MyMain.cpp)
target_link_libraries(MyCoolApplication QAppImageUpdate
					Qt5::Core 
					Qt5::Network
					Qt5::Widgets)
```

Where **MyCoolApplication** is the application that you are currently working with.

# Using CMake (When QAppImageUpdate installed to system)

QAppImageUpdate can be installed to the system with the following commands,

```
 $ git clone https://github.com/antony-jr/QAppImageUpdate
 $ cd QAppImageUpdate
 $ cmake .
 $ make -j$(nproc)
 $ sudo make install
```

Then it can be simply used using CMake's **find_package()**.

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

# Get QAppImageUpdate
find_package(QAppImageUpdate)

add_executable(MyCoolApplication MyMain.cpp)
target_link_libraries(MyCoolApplication QAppImageUpdate
					Qt5::Core 
					Qt5::Network
					Qt5::Widgets)

```

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

# Disable GUI Support

QAppImageUpdate gives a basic gui implementation, if you don't want it use this.

In QMake ,

```
 $ qmake "CONFIG+=NO_GUI" [ProjectFolder]
```

and 

In CMake ,

```
 $ cmake -DNO_GUI=ON [ProjectFolder]
```


# Enable Torrent Update Feature

As of v2.0 QAppImageUpdate supports updating AppImages via Bittorrent protocol.

**But you should have torrent rasterbar installed, At least v1.2.8 is needed for QAppImageUpdate.** 

> NOTE: Torrent Rasterbar v2.x is not supported yet. 



In QMake ,

```
 $ qmake "CONFIG+=DECENTRALIZED_UPDATE_ENABLED" [ProjectFolder]
```

and 

In CMake ,

```
 $ cmake -DDECENTRALIZED_UPDATE_ENABLED=ON [ProjectFolder]
```
