---
id: version-1.1.9-AppImageDeltaRevisionerExample
title: Updating an AppImage
sidebar_label: Updating an AppImage
original_id: AppImageDeltaRevisionerExample
---

This guide Demonstrates how to use the *AppImageUpdaterBridge* APIs for updating a single AppImage file.
This example parses the path from the program arguments , And uses the *[AppImageDeltaRevisioner](ClassAppImageDeltaRevisioner.html)* class to perform the actual delta update.

## main.cpp

```
#include <QCoreApplication>
#include <QDebug>
#include <AppImageUpdaterBridge>

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	using AppImageUpdaterBridge::AppImageDeltaRevisioner;
	QCoreApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	AppImageDeltaRevisioner DRevisioner(AppImagePath);
	QObject::connect(&DRevisioner , &AppImageDeltaRevisioner::finished ,
        [&](QJsonObject newVersionDetails , QString oldVersionPath){
		(void)oldVersionPath;
		qInfo() << "New Version Details:: " << newVersionDetails;
		app.quit();
	});
	QObject::connect(&DRevisioner , &AppImageDeltaRevisioner::error ,
        [&](short e){
		qInfo() << "error:: " << AppImageUpdaterBridge::errorCodeToString(e);
		app.quit();
	});
	DRevisioner.setShowLog(true); // Display log?
	
	DRevisioner.start(); /* Start the update. */
	return app.exec();
}
 
```

## update.pro

```
include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri)
TEMPLATE = app
TARGET = update
SOURCES += main.cpp
```

## Compilation and Execution

```
 $ mkdir build
 $ cd build
 $ qmake ..
 $ make -j$(nproc)
 $ ./update some.AppImage
```

A advanced verison of this program has been implemented in the examples directory.
