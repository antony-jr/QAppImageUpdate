---
id: ClassAppImageUpdaterDialog
title: Class AppImageUpdaterDialog
sidebar_label: Class AppImageUpdaterDialog
---

|	    |	        	                                       |		
|-----------|----------------------------------------------------------|
|  Header:  | #include < AppImageUpdaterDialog >                       |
|   qmake:  | include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri) |
|Inherits:  | [QObject](http://doc.qt.io/qt-5/qobject.html)            |
|Namespace: | **AppImageUpdaterBridge**


> **Important**: AppImageUpdaterDialog is under AppImageUpdaterBridge namespace , Make sure to include it.

**AppImageUpdaterDialog** can be used to use the delta revisioner in a nice confined Qt Widget.
Its a quick and easy to implement gui for the delta revisioner which can be handy at times.

All methods in this class is [reentrant](https://doc.qt.io/qt-5/threads-reentrancy.html) and thread safe.

## Public Functions

|                                                                                                                |
|----------------------------------------------------------------------------------------------------------------|
| AppImageUpdaterDialog(QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)                |
| AppImageUpdaterDialog(const QString&, QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)|
| AppImageUpdaterDialog(QFile\*, QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)       |

## Slots

| Return Type  | Name |
|------------------------------|-------------------------------------------|
| **void** | [init(void)](#void-startvoid) |
| **void** | [setAppImage(const QString&)](#void-setappimageconst-qstring) |
| **void** | [setAppImage(QFile *)](#void-setappimageqfile) |

## Signals

| Return Type  | Name |
|--------------|------------------------------------------------|
| void | [started(void)](#void-startedvoid) |
| void | [canceled(void)](#void-canceledvoid) |
| void | [finished(QJsonObject)](#void-finishedqjsonobject-qstring) |
| void | [error(short)](#void-errorshort) |
| void | quit(void) |
| void | requiresAuthorization(QString, short, QString) |

## Flags

| Name | Meaning | Value |
|------|---------|-------|


## Member Functions Documentation

### void start(void)
<p align="right"> <b>[SLOT]</b> </p>

Starts the updater.
Emits **started()** signal when starts.


### void cancel(void)
<p align="right"> <b>[SLOT]</b> </p>

Cancels the update.
Emits **canceled()** signal when cancel was successfull.


### void setAppImage(const QString&)
<p align="right"> <b>[SLOT]</b> </p>

Sets the AppImage Path as the given **QString**.


### void setAppImage(QFile *)
<p align="right"> <b>[SLOT]</b> </p>

Sets the given **QFile\*** as the AppImage itself.

### void setShowLog(bool)
<p align="right"> <b>[SLOT]</b> </p>

Turns on and off the log printer.

> Note: logger signal will be emitted all the time if the library is compiled with LOGGING_DISABLED undefined ,
setShowLog will not affect this activity at all , But setShowLog will print these log messages
if set to true.

### void setOutputDirectory(const QString&)
<p align="right"> <b>[SLOT]</b> </p>

Writes the new version of the AppImage to the given Output directory , Assuming the given QString a directory path.
The default is the old version AppImage's directory.

### void started(void)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is started successfully.

### void canceled(void)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is canceled successfully.

### void finished(QJsonObject)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is finished successfully. The given *QJsonObject* has the details of the new version
of the AppImage.

The *QJsonObject* will follow the following format with respect to json ,
	
    {
        "AbsolutePath" : Absolute path of the new version of the AppImage ,
        "Sha1Hash"     : Sha1 hash of the new version of the AppImage
    }

> Note: If the absolute path of the new version of the AppImage is same as the old version then
it could mean that there were no updates needed.

### void error(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](https://antony-jr.github.io/AppImageUpdaterBridge/docs/AppImageUpdaterBridgeErrorCodes.html).

### requiresAuthorization(QString, short, QString)

The first **QString** is the error string , the second is the error code returned by the delta revisioner.
The third **QString** is the path to the current operating appimage. 
This signal is emitted when the current operating appimage update failed due to some permission errors.

### void quit()
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when its suitable to quit the application in case you are using this for self update.
