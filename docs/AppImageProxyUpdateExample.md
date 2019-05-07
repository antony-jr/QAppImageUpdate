---
id: AppImageDeltaRevisionerProxyExample
title: Updating an AppImage using Proxy
sidebar_label: Updating an AppImage using Proxy
---

This guide Demonstrates how to use the *AppImageUpdaterBridge* APIs for updating a single AppImage file through a given **proxy**.
This example parses the path from the program arguments , And uses the *[AppImageDeltaRevisioner]()* class to
perform the actual delta update.

## main.cpp

```
#include <QCoreApplication>
#include <QDebug>
#include <AppImageUpdaterBridge>
#include <QNetworkProxy> 

int main(int ac , char **av)
{
	if(ac == 1){
		qInfo() << "Usage: " << av[0] << " [APPIMAGE PATH]";
		return 0;	
        }
	
	using AppImageUpdaterBridge::AppImageDeltaRevisioner;
	using AppImageUpdaterBridge::errorCodeToString;
        QCoreApplication app(ac , av);
 	QString AppImagePath = QString(av[1]);

	/* Set proxy settings */
	QNetworkProxy proxy;
	/* For this demo , we assume we use a local tor instance. */
	proxy.setType(QNetworkProxy::Socks5Proxy);
	proxy.setHostName("127.0.0.1");
	proxy.setPort(9050);

	AppImageDeltaRevisioner DRevisioner(AppImagePath);
	QObject::connect(&DRevisioner , &AppImageDeltaRevisioner::finished ,
        [&](QJsonObject newVersionDetails , QString oldVersionPath){
		(void)oldVersionPath;
		qInfo() << "New Version Details:: " << newVersionDetails;
		app.quit();
        });
	QObject::connect(&DRevisioner , &AppImageDeltaRevisioner::error ,
        [&](short e){
		qInfo() << "error:: " << errorCodeToString(e);
		app.quit();
        });
       /*
        * Enable this if you want to print the log messages in 
        * the standard output.
       */
	DRevisioner.setShowLog(true);
	
	/* Using proxy. */
        DRevisioner.setProxy(proxy);
	
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
