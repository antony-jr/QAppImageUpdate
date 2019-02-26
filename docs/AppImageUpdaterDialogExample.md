---
id: AppImageUpdaterDialogExample
title: Updating an AppImage with GUI
sidebar_label: Updating an AppImage with GUI
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
        QApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	AppImageUpdaterDialog UWidget(AppImagePath);
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
        /*
         * Enable this if you want to print the log messages in 
         * the standard output.
        */
	UWidget.setShowLog(true);
	
	UWidget.init(); /* Start the update. */
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
