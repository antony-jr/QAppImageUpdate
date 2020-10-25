---
id: version-2.0.0-ProxyExample
title: Updating an AppImage using Proxy
sidebar_label: Updating an AppImage using Proxy
original_id: ProxyExample
---

This guide Demonstrates how to use the *QAppImageUpdate* API for updating a single AppImage file through a given **proxy**.
This example parses the path from the program arguments.

## main.cpp

```
#include <QCoreApplication>
#include <QDebug>
#include <QAppImageUpdate>
#include <QNetworkProxy> 

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	QCoreApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	/* Set proxy settings */
	QNetworkProxy proxy;
	/* For this demo, we assume we use a local tor instance. */
	proxy.setType(QNetworkProxy::Socks5Proxy);
	proxy.setHostName("127.0.0.1");
	proxy.setPort(9050);

	QAppImageUpdate updater(AppImagePath);
	QObject::connect(&updater , &QAppImageUpdate::finished ,
        [&](QJsonObject info , short action){
		if(action == QAppImageUpdate::Action::Update) {
			qInfo() << "New Version Details:: " << info;
			app.quit();
		}
	});
	QObject::connect(&updater , &QAppImageUpdate::error ,
        [&](short e, short action){
		if(action == QAppImageUpdate::Action::Update) {
			qInfo() << "error:: " << QAppImageUpdate::errorCodeToString(e);
			app.quit();
		}
	});
	updpater.setShowLog(true); // Display log
        
	/* Using proxy. */
	updater.setProxy(proxy);
	
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
