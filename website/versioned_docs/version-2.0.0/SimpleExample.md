---
id: version-2.0.0-SimpleExample
title: Updating an AppImage
sidebar_label: Updating an AppImage
original_id: SimpleExample
---

This guide Demonstrates how to use the *QAppImageUpdate* APIs for updating a single AppImage file.
This example parses the path from the program arguments.

## main.cpp

```
#include <QCoreApplication>
#include <QDebug>
#include <QAppImageUpdate>

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	QCoreApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	QAppImageUpdate updater(AppImagePath);
	QObject::connect(&updater , &QAppImageUpdate::finished ,
        [&](QJsonObject newVersionDetails , short action){
		if(action == QAppImageUpdate::Action::Update) {
			qInfo() << "New Version Details:: " << newVersionDetails;
			app.quit();
		}
	});
	QObject::connect(&updater, &QAppImageUpdate::error ,
        [&](short e){
		qInfo() << "error:: " << QAppImageUpdate::errorCodeToString(e);
		app.quit();
	});
	updater.setShowLog(true); // Display log?
	
	updater.start(QAppImageUpdate::Action::Update); /* Start the update. */
	return app.exec();
}
 
```

## update.pro

```
include(QAppImageUpdate/QAppImageUpdate.pri)
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
