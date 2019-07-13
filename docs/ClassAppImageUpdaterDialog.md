---
id: ClassAppImageUpdaterDialog
title: Class AppImageUpdaterDialog
sidebar_label: Class AppImageUpdaterDialog
---

|	        |   	        	                                       |		
|-----------|----------------------------------------------------------|
|  Header:  | #include < AppImageUpdaterDialog >                       |
|   qmake:  | include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri) |
|Inherits:  | [QDialog](http://doc.qt.io/qt-5/qdialog.html)            |
|Namespace: | **AppImageUpdaterBridge**                                |


> **Important**: AppImageUpdaterDialog is under AppImageUpdaterBridge namespace , Make sure to include it.

**AppImageUpdaterDialog** can be used to use the delta revisioner in a nice confined Qt Widget.
Its a quick and easy to implement gui for the delta revisioner which can be handy at times.

All methods in this class is [reentrant](https://doc.qt.io/qt-5/threads-reentrancy.html) and thread safe.

## Public Functions

|                                                                                                                |
|----------------------------------------------------------------------------------------------------------------|
| [AppImageUpdaterDialog(QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)](#appimageupdaterdialogqpixmap-img-qpixmap-qwidget-parent-nullptr-int-flags-default)           |

## Slots

| Return Type  | Name |
|------------------------------|-------------------------------------------|
| **void** | [init(AppImageDeltaRevisioner \*revisioner = nullptr, const QString &applicationName = QApplication::applicationName())](#void-init-appimagedeltarevisioner-classappimagedeltarevisionerhtml-revisioner-nullptr-const-qstring-applicationname-qapplication-applicationname) |

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
| AppImageUpdaterDialog::NoRemindMeLaterButton            |  Do not show 'Remind me later' in the confirmation dialog.     |  0x80 |
| AppImageUpdaterDialog::NoSkipThisVersionButton          |  Do not show 'Skip This Version' in the confirmatino dialog.   |  0x100|
| AppImageUpdaterDialog::Default                          |  Give the default combination of flags                         |  0x1df|


> The Default flag enables ShowProgressDialog, ShowBeforeProgress, ShowUpdateConfirmationDialog, ShowFinishedDialog,
> ShowErrorDialog, NotifyWhenNoUpdateIsAvailable, NoRemindMeLaterButton, NoSkipThisVersionButton


## Member Functions Documentation

### AppImageUpdaterDialog(QPixmap img = QPixmap(), QWidget \*parent = nullptr, int flags = Default)

Default constructor for *AppImageUpdaterDialog* , uses the given **QPixmap**(img) as the icon throughout the
update process.

> The Default flag enables ShowProgressDialog, ShowBeforeProgress, ShowUpdateConfirmationDialog, ShowFinishedDialog,
> ShowErrorDialog, NotifyWhenNoUpdateIsAvailable, NoRemindMeLaterButton, NoSkipThisVersionButton


### void init([AppImageDeltaRevisioner](ClassAppImageDeltaRevisioner.html) \*revisioner = nullptr , const QString &applicationName = QApplication::applicationName())
<p align="right"> <b>[SLOT]</b> </p>

Starts the updater using the given [AppImageDeltaRevisioner](ClassAppImageDeltaRevisioner.html), You must set the needed settings for the given delta revisioner.
Emits **started()** signal when starts the actuall update,(i.e) After finishing update check and getting update confirmation.

The application name is used in the update confirmation dialog.

> If the revisioner is not given then a new instance of AppImageDeltaRevisioner is created and the AppImage path is 
> guessed.

> You must disconnect all slots to this given AppImageDeltaRevisioner to avoid any collision before giving it to this method. 

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
See [error codes](AppImageUpdaterBridgeErrorCodes.html).
The **QString** is the error in layman terms.

### requiresAuthorization(QString, short, QString)

The first **QString** is the error string , the second is the error code returned by the delta revisioner.
The third **QString** is the path to the current operating appimage. 
This signal is emitted when the current operating appimage update failed due to some permission errors.

### void quit()
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when its suitable to quit the application in case you are using this for self update.
