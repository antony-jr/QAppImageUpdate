---
id: version-1.1.9-AppImageUpdaterDialogExample
title: Updating an AppImage with GUI
sidebar_label: Updating an AppImage with GUI
original_id: AppImageUpdaterDialogExample
---

This guide demonstrates on using the AppImage updater dialog to update appimages through 
graphical user interface.

## main.cpp

```
#include <QApplication>
#include <QDebug>
#include <AppImageUpdaterBridge>
#include <AppImageUpdaterDialog>

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	using AppImageUpdaterBridge::AppImageUpdaterDialog;
	using AppImageUpdaterBridge::AppImageDeltaRevisioner;
	QApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	AppImageDeltaRevisioner DRev(AppImagePath);
	DRev.setShowLog(true); // Display log?
	
	AppImageUpdaterDialog UWidget;
	QObject::connect(&UWidget , &AppImageUpdaterDialog::finished ,
        [&](QJsonObject newVersionDetails){
		qInfo() << "New Version Details:: " << newVersionDetails;
		app.quit();
	});
	QObject::connect(&UWidget , &AppImageUpdaterDialog::error ,
        [&](QString eStr , short e){
		qInfo() << "error(" << e "):: " << eStr;
		app.quit();
	});	
	UWidget.init(&DRev); /* Start the update using GUI */
	return app.exec();
}
 
```

## update.pro

```
include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri)
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
