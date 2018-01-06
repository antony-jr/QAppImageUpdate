---
id: UpdatingFromAppImageInformation
title: Update your AppImage right from your AppImage Information
sidebar_label: Updating From AppImage Update Information
---


This is a very small example on how to check for updates using your current **AppImage** itself   
Please see that you must follow the [AppImageSpec](https://github.com/AppImage/AppImageSpec/blob/master/draft.md#update-information) and must built the **AppImage** with the **Update Information embeded into it**. Please ask the **[AppImage](https://appimage.org)** Dev for more information about this.


**main.cpp**

```
#include <QCoreApplication>
#include "AppImageUpdaterBridge/AIUpdaterBridge.hpp"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    AIUpdaterBridge Bridge;

    QObject::connect(&Bridge, &AIUpdaterBridge::updatesAvailable,
    [&](QString appimage, QString sha1) {
        //Start the update // Ask the user a choose about Update here!
        Bridge.start();
    });
    QObject::connect(&Bridge, &AIUpdaterBridge::updateFinished,
    [&](QString appImage, QString SHA1) {
        qDebug() << "Updated " << appImage << " to " << SHA1;
        qDebug() << "Please Restart AppImage!";
        app.quit();
        return;
    });
    QObject::connect(&Bridge, &AIUpdaterBridge::progress,
    [&](float percentage, qint64 bytesRecived, qint64 bytesTotal, 
        double speed, const QString& unit) {
        qDebug() << "Downloaded:: " << bytesRecived << "bytes of " << bytesTotal
                 <<" Percentage:: " << percentage << " at " << speed << unit;
        return;
    });
    QObject::connect(&Bridge, &AIUpdaterBridge::noUpdatesAvailable,
    [&](QString appImage, QString SHA1) {
        qDebug() << "AppImage Version(" << SHA1 << ")";
        qDebug() << "Starting Application!"; // Run Your App Here
        app.quit();
        return;
    });
    QObject::connect(&Bridge, &AIUpdaterBridge::error,
    [&](QString appImage, short errorCode) {
        qDebug() << "ERROR CODE:: " << errorCode;
        Bridge.quit();
        app.quit();
        return;
    });
    Bridge.doDebug(true); // set this false to keep quite
    // use argv or constant filenames to set the AppImage path.
    Bridge.setAppImageUpdateInformation("YourAppImage.AppImage"); // This is a little sync
    return app.exec();
}

```


**CMakeLists.txt**

```
CMAKE_MINIMUM_REQUIRED( VERSION 3.2)
project(Updater)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtCore library
find_package(Qt5Core)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(AppImageUpdaterBridge)
# include subdirectories in the right order
add_subdirectory(AppImageUpdaterBridge)
add_executable(Updater main.cpp)
target_link_libraries(Updater AIUpdaterBridge Qt5::Core)
```

## Compilation and Execution

```
 $ mkdir build
 $ cd build
 $ cmake ..
 $ make -j4
 $ ./Updater
```
