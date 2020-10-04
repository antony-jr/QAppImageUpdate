---
id: GUIExample
title: Updating an AppImage with GUI
sidebar_label: Updating an AppImage with GUI
---

This guide demonstrates on using the QAppImageUpdate to update appimages through 
graphical user interface.

## main.cpp

```
#include <QApplication>
#include <QDebug>
#include <QAppImageUpdate>

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	QApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	QAppImageUpdate updater(AppImagePath);
	updater.setShowLog(true); // Display log?
	
	QObject::connect(&updater , &QAppImage::finished ,
        [&](QJsonObject info, short action){
		if(action == QAppImageUpdate::Action::UpdateWithGUI) {
			qInfo() << "New Version Details:: " << info;
			app.quit();
		}
	});
	QObject::connect(&updater , &QAppImageUpdate::error ,
        [&](short e, short action){
		if(action == QAppImageUpdate::Action::UpdateWithGUI) {
			qInfo() << "error(" << e "):: "
                                << QAppImageUpdate::errorCodeToDescriptionString(e);
			app.quit();
		}
	});	

	updater.start(QAppImageUpdate::Action::UpdateWithGUI);
	return app.exec();
}
 
```

## update.pro

```
include(QAppImageUpdate/QAppImageUpdate.pri)
TEMPLATE = app
TARGET = updategui
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
