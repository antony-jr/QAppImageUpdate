---
id: ClassAIUpdaterBridge
title: Class AIUpdaterBridge
sidebar_label: Class AIUpdaterBridge
---

|	        |	    	                                           |		
|-----------|------------------------------------------------------|
|  Header:  | #include "AppImageUpdaterBridge/AIUpdaterBridge.hpp" |
|   qmake:  | QT += core network concurrent                          |
|	        | HEADERS += AppImageUpdaterBridge/AIUpdaterBridge.hpp |
|Inherits:  | [QObject](http://doc.qt.io/qt-5/qobject.html)        |


## Public Functions

|               |                                                               |
|--------------	|-------------------------------------------------------------	|
| **explicit** 	| AIUpdaterBridge(QNetworkAccessManager *toUseManager = NULL) 	|
| **explicit** 	| AIUpdaterBridge(const QString& AppImagePath) 	|
| **explicit** 	| AIUpdaterBridge(const QJsonObject& config) 	|
| **void**      | setChangelogURL(const QUrl& url)				|
| const QString&| getChangelog(void)						|
| **void** 	| setAppImageUpdateInformation(const QString& AppImagePath) 	|
| **void** 	| setAppImageUpdateInformation(const QJsonObject& config) 	|
| **void** 	| doDebug(bool choose) 	|


## Slots

|               |                     |
|---------------|---------------------|
| **void**  	| startUpdating(void) |
| **void**      | stopUpdating(void)  |

## Signals

|           |                                                                                                            |
|----------	|------------------------------------------------------------------------------------------------------------|
| **void**  | stopped(void)                                                     |
| **void** 	| updatesAvailable(const QString& AppImage, const QString& SHA1) 	|
| **void** 	| noUpdatesAvailable(const QString& AppImage, const QString& SHA1) 	|
| **void** 	| updateFinished(const QString& AppImage , const QString& SHA1) 	|
| **void** 	| progress(float percent, qint64 bytesRecived , qint64 bytesTotal , double speed , const QString& unit) 	|
| **void** 	| void error(const QString& AppImage, short **[errorCode](AppImageUpdaterBridgeErrorCodes.md)**) 	|
| **void** 	| doDebug(bool choose) 	|


## Member Functions Documentation

### explicit AIUpdaterBridge(QNetworkAccessManager *toUseManager = NULL)

Constructs the class and sets the **QNetworkAccessManager** if the user gives one , else allocates one.

### explicit AIUpdaterBridge(const QString& AppImagePath)

This is a overloaded function.   
Constructs the class and extracts the **Update information** from the **AppImage**.

### void setAppImageUpdateInformation(const QString& AppImagePath)

Extracts the **Update information** from the **AppImage**.

### void setAppImageUpdateInformation(const QJsonObject& config)

This is a overloaded function.   
Extracts the **Update information** from the given **QJsonObject**.

### void doDebug(bool choose)

Sets Debuging.

### void startUpdating(void)
<p align="right"> <b>[SLOT]</b> </p>

Starts the Update , Does nothing if required components for updates are missing.

### void stopUpdating(void)
<p align="right"> <b>[SLOT]</b> </p>

Stops the Updater. Emits **void stopped(void)** signal when this is successfull.

### void stopped(void)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when **void stopUpdating(void)** is successfull. In other words , Emitted when the installation is aborted safely!

### void updatesAvailable(const QString& AppImage, const QString& SHA1)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when **new updates** are available.

### void noUpdatesAvailable(const QString& AppImage, const QString& SHA1)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when **no new updates** are available.

### void updateFinished(const QString& AppImage , const QString& SHA1)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when a **update** was **successfull**!

### void progress(float percent, qint64 bytesRecived , qint64 bytesTotal , double speed , const QString& unit)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted for zsync download progress.

### void error(const QString& AppImage, short **[errorCode](AppImageUpdaterBridgeErrorCodes.md)**)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when something goes wrong. Please refer the **[error codes](AppImageUpdaterBridgeErrorCodes.md)** to know more.
