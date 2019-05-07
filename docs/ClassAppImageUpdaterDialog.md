---
id: ClassAppImageUpdaterDialog
title: Class AppImageUpdaterDialog
sidebar_label: Class AppImageUpdaterDialog
---

|	        |   	        	                                       |		
|-----------|----------------------------------------------------------|
|  Header:  | #include < AppImageUpdaterDialog >                       |
|   qmake:  | include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri) |
|Inherits:  | [QObject](http://doc.qt.io/qt-5/qobject.html)            |
|Namespace: | **AppImageUpdaterBridge**                                |


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
| **void** | [init(void)](#void-initvoid) |
| **void** | [setAppImage(const QString&)](#void-setappimageconst-qstring) |
| **void** | [setAppImage(QFile \*)](#void-setappimageqfile) |
| **void** | [setShowLog(bool)](#void-setshowlogbool) |
| **void** | [setProxy(const QNetworkProxy&)](#void-setproxyconst-qnetworkproxy-https-docqtio-qt-5-qnetworkproxyhtml) |

## Signals

| Return Type  | Name |
|--------------|------------------------------------------------|
| void | [started(void)](#void-startedvoid) |
| void | [canceled(void)](#void-canceledvoid) |
| void | [finished(QJsonObject)](#void-finishedqjsonobject-qstring) |
| void | [error(QString , short)](#void-errorqstring-short) |
| void | [quit(void)](#void-quit) |
| void | [requiresAuthorization(QString, short, QString)](#requiresauthorizationqstring-short-qstring) |

## Flags

| Variable Name                                           |  Meaning                                                       | Value |
|---------------------------------------------------------|----------------------------------------------------------------|-------|
| AppImageUpdaterDialog::ShowProgressDialog               |  Show the progress dialog during update.                       |  0x1  |
| AppImageUpdaterDialog::ShowBeforeProgress               |  Show the progress dialog before the update starts.            |  0x2  |
| AppImageUpdaterDialog::ShowUpdateConfirmationDialog     |  Show a update confirmation dialog.                            |  0x4  |
| AppImageUpdaterDialog::ShowFinishedDialog               |  Show a message box when update is finished.                   |  0x8  |
| AppImageUpdaterDialog::ShowErrorDialog                  |  Show a error message box when update is errored.              |  0x10 |
| AppImageUpdaterDialog::AlertWhenAuthorizationIsRequired |  Emit **requiresAuthorization** when authorization is required.|  0x20 |
| AppImageUpdaterDialog::NotifyWhenNoUpdateIsAvailable    |  Show a message box when there was no update.                  |  0x40 |


## Member Functions Documentation

### AppImageUpdaterDialog(QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)

Default constructor for *AppImageUpdaterDialog* , uses the given **QPixmap**(img) as the icon throughout the
update process.

### AppImageUpdaterDialog(const QString &path , QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)

Overloaded constructor. Sets the given **QString**(path) as the AppImage path.


### AppImageUpdaterDialog(QFile *AppImage , QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)

Overloaded constructor.Sets the given **QFile** as the AppImage itself.


### void init(void)
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


### void setAppImage(QFile \*)
<p align="right"> <b>[SLOT]</b> </p>

Sets the given ** QFile\* ** as the AppImage itself.

### void setShowLog(bool)
<p align="right"> <b>[SLOT]</b> </p>

Turns on and off the log printer.

### void setProxy(const [QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html)&)
<p align="right"> <b>[SLOT]</b> </p>

Set proxy for the updater which is used for all network communications.


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

### void error(QString , short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](https://antony-jr.github.io/AppImageUpdaterBridge/docs/AppImageUpdaterBridgeErrorCodes.html).
The **QString** is the error in layman terms.

### requiresAuthorization(QString, short, QString)

The first **QString** is the error string , the second is the error code returned by the delta revisioner.
The third **QString** is the path to the current operating appimage. 
This signal is emitted when the current operating appimage update failed due to some permission errors.

### void quit()
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when its suitable to quit the application in case you are using this for self update.
