---
id: version-1.1.9-PluginInterface
title: AppImage Updater Bridge Plugin Interface
sidebar_label: Qt Plugin Interface
original_id: PluginInterface
---

A plugin interface is a specification of public slots and signals that can be used to access 
the functionality of the plugin itself. The interface is not bound to any specific programming 
language and easily translates to any Qt bindings and programming language.

All slots are [reentrant](https://doc.qt.io/qt-5/threads-reentrancy.html) and thread safe.

Please refer the C++ documentation for info on how the slots act.

> IMPORTANT: You have to start your Qt event loop for AppImageUpdaterBridge to function.

### A note on Data Types

Since plugins are not C++ specific, The data types are vaguely defined.
In dynamic languages like python, you can just use native data types. (i.e) For QString, you can use str or QString from your Qt binding itself.

I'm not sure about other Qt bindings, So help is much welcomed.

## Slots

| Name | Description                        |
|------|------------------------------------|
| [start()](#start) | Starts the update.    |
| [cancel()](#cancel) | Cancels current update process. |
| [setAppImage(QString)](#setappimage-qstring) | Assume the given string as path to AppImage to update. |
| [setShowLog(bool)](#setshowlogbool) | If the given boolean is true then prints log. |
| [setOutputDirectory(QString)](#setoutputdirectory-qstring) | Set the output directory as given string. | 
| [setProxy(QNetworkProxy)](#setproxyconst-qnetworkproxy-https-docqtio-qt-5-qnetworkproxyhtml) | Use proxy as given in QNetworkProxy object. |
| [checkForUpdate()](#checkforupdate) | Checks for new update. |
| [clear()](#clear) | Clears internal cache and stores. | 

## Signals

| Name | Description   | 
|------|---------------|
| [started()](#started) | Emitted when the update is actually started. |
| [canceled()](#canceled) | Emitted when the update is canceled. |
| [finished(QJsonObject , QString)](#finishedqjsonobject-qstring) | Emitted when update finishes. |
| [updateAvailable(bool, QJsonObject)](#updateavailablebool-qjsonobject) | Emitted when checkForUpdate() is called. |
| [error(short)](#errorshort) | Emitted when some error occurs. |
| [progress(int, qint64, qint64, double, QString)](#progressint-percentage--qint64-bytesreceived--qint64-bytestotal--double-speed--qstring-speedunits) | Emitted on progress of update. See here for more information. |
| [logger(QString, QString)](#loggerqstring-qstring) | See here for more information. |


## Documentation

### start()
<p align="right"> <b>[SLOT]</b> </p>

Starts the updater.
Emits **started()** signal when starts.


> Minor Note: You don't have to worry about anything if you called checkForUpdate or getAppImageEmbededInformation 
slots before start , Don't worry about overheads too , Since when you call checkForUpdate slot , The information
is cached and when start slot is called again , it will be faster than normal. 

> Important Note: You should also call clear and set the settings again if you want to clear the cache.

### cancel()
<p align="right"> <b>[SLOT]</b> </p>

Cancels the update.
Emits **canceled()** signal when cancel was successfull.


### setAppImage(QString)
<p align="right"> <b>[SLOT]</b> </p>

Sets the AppImage Path as the given **QString**.


### setShowLog(bool)
<p align="right"> <b>[SLOT]</b> </p>

Turns on and off the log printer.

> Note: logger signal will be emitted all the time if the library is compiled with LOGGING_DISABLED undefined,
setShowLog will not affect this activity at all, But setShowLog will print these log messages
if set to true.

### setOutputDirectory(QString)
<p align="right"> <b>[SLOT]</b> </p>

Writes the new version of the AppImage to the given Output directory , Assuming the given QString a directory path.
The default is the old version AppImage's directory.


### setProxy([QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html))
<p align="right"> <b>[SLOT]</b> </p>

Sets the given [QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html) as the proxy

### checkForUpdate()
<p align="right"> <b>[SLOT]</b> </p>

Checks update for the current operating AppImage.
emits **updateAvailable(bool , QJsonObject)** , Where the *bool*  will be **true** if the AppImage
needs update. The QJsonObject in the signal will have the details of the current operating
AppImage.


### clear()
<p align="right"> <b>[SLOT]</b> </p>

Clears all internal **cache** and stores.


### started()
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is started successfully.

### canceled()
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is canceled successfully.

### finished(QJsonObject , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is finished successfully. The given *QJsonObject* has the details of the new version
of the AppImage and the given *QString* has the absolute path to the old versioin of the AppImage.

The *QJsonObject* will follow the folloing format with respect to json ,
	
    {
        "AbsolutePath" : "Absolute path of the new version of the AppImage" ,
        "Sha1Hash"     : "Sha1 hash of the new version of the AppImage"
    }

> Note: If the absolute path of the new version of the AppImage is same as the old version then
it could mean that there were no updates needed , You can however listen to the *updateAvailable*
signal to know the exact state of updates. You should call *checkForUpdate* and then call *start*
if updates were really available.


### updateAvailable(bool , QJsonObject)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when *[checkForUpdate()](#checkforupdate)* is called.
The given *bool* states if the operating AppImage needs update and the *QJsonObject* gives the details of 
the current operating AppImage.

The *QJsonObject* will follow the following format with respect to json , 
	
    {
        "AbsolutePath" : "The absolute path of the current operating AppImage" ,
        "Sha1Hash"     : "The Sha1 hash of the current operating AppImage" ,
        "RemoteSha1Hash" : "The Sha1 hash of the lastest AppImage" ,
        "ReleaseNotes" : "Release notes if available"
    }

### error(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](AppImageUpdaterBridgeErrorCodes.html).


### progress(int percentage , qint64 bytesReceived , qint64 bytesTotal , double speed , QString speedUnits)
<p align="right"> <b>[SIGNAL]</b> </p>

The updater's progress is emitted through this unified signal.

**Where** ,

| Variable       | Description                                                      |
|----------------|------------------------------------------------------------------|
| percentage     | % Finished revising the latest AppImage.                         |
| bytesReceived  | The received bytes of the latest AppImage.                       |
| bytesTotal     | The total bytes of the latest AppImage.                          |
| speed          | The transfer speed value.                                        |
| speedUnit      | The transfer speed unit(e.g. KiB/s , etc... ) for **speed**.     |

### logger(QString , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater issues a log message with the *first QString* as the log message and
the *second QString* as the path to the respective AppImage.

