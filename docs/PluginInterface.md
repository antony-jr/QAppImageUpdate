---
id: PluginInterface
title: QAppImageUpdate Plugin Interface
sidebar_label: Qt Plugin Interface
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

| Name                          | Description                     |
|-------------------------------|---------------------------------|
| [setGuiFlag(int)](#setguiflag-int) | Set GUI Flag.              |
| [setIcon(QByteArray)](#seticon-qbytearray) | Set Icon for GUI Update. |
| [start(short)](#start-short)  | Starts the given action.        |
| [cancel()](#cancel)           | Cancels current update process. |
| [setAppImagePath(QString)](#setappimage-qstring) | Assume the given string as path to AppImage to update. |
| [setAppImageFile(QFile\*)](#setappimage-qfile)   | Assum the given QFile as the AppImage to update. |
| [setShowLog(bool)](#setshowlogbool) | If the given boolean is true then prints log. |
| [setOutputDirectory(QString)](#setoutputdirectory-qstring) | Set the output directory as given string. | 
| [setProxy(QNetworkProxy)](#setproxyconst-qnetworkproxy-https-docqtio-qt-5-qnetworkproxyhtml) | Use proxy as given in QNetworkProxy object. |
| [getConstant(QString)](#getconstant-qstring) | Get the constant with respect to the string. |
| [getObject()](#getobject) | Get QObject to slots to connect to this plugin. |
| [clear()](#clear) | Clears internal cache and stores. | 
| [errorCodeToString(short)](#errorcodetostring-shortt) | Convert the given error code to string. |
| [errorCodeToDescriptionString(short)](#errorcodetodescriptionstring-short) | Convert the given error code to description. |

## Signals

| Name                              | Description                                  | 
|-----------------------------------|----------------------------------------------|
| [started(short)](#started-short)  | Emitted when a action is started.            |
| [canceled(short)](#canceled-short)| Emitted when a action is canceled.           |
| [finished(QJsonObject , short)](#finishedqjsonobject-qstring-short) | Emitted when a action is finished. |
| [error(short, short)](#error-short-short) | Emitted when some error occurs in an action. |
| [progress(int, qint64, qint64, double, QString, short)](#progressint-percentage--qint64-bytesreceived--qint64-bytestotal--double-speed--qstring-speedunits) | Emitted on progress of a action. |
| [logger(QString, QString)](#loggerqstring-qstring) | See here for more information. |


## Actions

| Variable Name                   | Value |
|---------------------------------|------ |
| Action::GetEmbeddedInfo         |   0   |
| Action::CheckForUpdate          |   1   |
| Action::Update                  |   2   |
| Action::UpdateWithTorrent       |   3   |
| Action::UpdateWithGUI           |   4   |
| Action::UpdateWithGUIAndTorrent |   5   |


You can use **getConstant(QString)** method of the plugin interface to get the value for a action.

```
   # PyQt5 Code
   plugin_instance.getConstant("Action::GetEmbeddedInfo") # = 0
```

## GUI Flags

| Variable Name                                  |  Meaning                                                        | Value |
|------------------------------------------------|-----------------------------------------------------------------|-------|
| GuiFlag::ShowProgressDialog                    |  Show the progress dialog during update.                        |  0x1  |
| GuiFlag::ShowBeforeProgress                    |  Show the progress dialog before the update starts.             |  0x2  |
| GuiFlag::ShowUpdateConfirmationDialog          |  Show a update confirmation dialog.                             |  0x4  |
| GuiFlag::ShowFinishedDialog                    |  Show a message box when update is finished.                    |  0x8  |
| GuiFlag::ShowErrorDialog                       |  Show a error message box when update is errored.               |  0x10 |
| GuiFlag::NoShowErrorDialogOnPermissionErrors   |  Do not show error dialog on permission errors. Only emit error.|  0x20 |
| GuiFlag::NotifyWhenNoUpdateIsAvailable         |  Show a message box when there was no update.                   |  0x40 |
| GuiFlag::NoRemindMeLaterButton                 |  Do not show 'Remind me later' in the confirmation dialog.      |  0x80 |
| GuiFlag::NoSkipThisVersionButton               |  Do not show 'Skip This Version' in the confirmatino dialog.    |  0x100|
| GuiFlag::NoConfirmTorrentUsage                 |  Do not confirm when torrent update is used.                    |  0x200|
| GuiFlag::Default                               |  Give the default combination of flags                          |  0x1df|


You can use **getConstant(QString)** method of the plugin interface to get the value for a gui flag same as with
actions.

```
   # PyQt5 Code
   plugin_instance.getConstant("GuiFlag::NoConfirmTorrentUsage") # = 0x200
```


> The Default flag enables ShowProgressDialog, ShowBeforeProgress, ShowUpdateConfirmationDialog, ShowFinishedDialog,
> ShowErrorDialog, NotifyWhenNoUpdateIsAvailable, NoRemindMeLaterButton, NoSkipThisVersionButton


## Documentation


### setGUIFlag(int flag)
<p align="right"> <b>[SLOT]</b> </p>

Set the given integer as the flag for GUI update if used.

### setIcon(QByteArray icon)
<p align="right"> <b>[SLOT]</b> </p>

Set the given icon which is saved as QByteArray. You have to save your ```QPixmap``` as a QByteArray, see [the official docs](https://doc.qt.io/qt-5/qpixmap.html#save-1).

```
 /// C++ Ref for saving QPixmap as QByteArray
 QPixmap pixmap;
 QByteArray bytes;
 QBuffer buffer(&bytes);
 buffer.open(QIODevice::WriteOnly);
 pixmap.save(&buffer, "PNG"); // writes pixmap into bytes in PNG format
```

### start(short action)
<p align="right"> <b>[SLOT]</b> </p>

Starts the given action in the argument.
Emits **started(short action)** signal when starts.

The short integer should be one of the possible actions as given in the Actions table.
One has to use **getConstant(QString)** method to get a constant.


> Please start your QApplication event loop to get effect from the updater.


```
   # PyQt5 code
   updater = plugin_loader.instance()
   updater.setAppImagePath("Some.AppImage") 

   def handleFinish(jsonObj, action):
      if action == updater.getConstant("Action::CheckForUpdate"):
         if jsonObj["UpdateAvailable"].toBool():
            updater.start(updater.getConstant("Action::Update")
         return;
      if action == updater.getConstant("Action::Update")
         app.quit()

   updater.finished.connect(handleFinish)

   updater.start(updater.getConstant("Action::CheckForUpdate"))
```

> Important Note: You should also call clear and set the settings again if you want to clear the cache.


### cancel()
<p align="right"> <b>[SLOT]</b> </p>

Cancels the current action.
Emits **canceled(short)** signal when cancel for a action was successfull.

### setAppImagePath(QString)
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

> WARNING: when using torrent support, only HTTP and SOCKS5 proxy is supported.

### clear()
<p align="right"> <b>[SLOT]</b> </p>

Clears all internal **cache** and stores.


### started(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when a action is started, with the argument denoting the action which is started.

### canceled(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when a action is canceled, with the argument denoting the action which is canceled.


### finished(QJsonObject , short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when a action is finished successfully. The given *QJsonObject* is variable and it is dependent 
on different actions.


The *QJsonObject* will follow the following format with respect to json for ```Action::GetEmbeddedInfo``` action,
    
    {
        "IsEmpty": <True if the update information is empty>,
        "FileInformation": {
        	"AppImageFilePath": <Local AppImage Absolute Path>,
                "AppImageSHA1Hash":<Sha1 hash of the local AppImage>
	},
        "UpdateInformation": {
                "transport" : <zsync/gh-releases-zsync/bintray-zsync>,
                <Based on the transport this part changes>
	}
    }

**gh-releases-zsync** , 

    {
     "transport" : "gh-releases-zsync",
     "username"  : <Username of the author>,
     "repo"      : <Name of the repo>,
     "tag"       : <continuous / latest / other>,
     "filename"  : <Latest AppImage Zsync filename with wild card>
    }

**bintray-zsync** , 

    {
        "transport" : "bintray-zsync" ,
        "username"  : <Username of the author>,
        "repo"      : <Name of the repo>,
        "packageName" : <Name of the package in bintray>,
        "filename" : <Name of the latest AppImage Zsync file>
    }

**zsync**,

    {
        "transport" : "zsync",
        "zsyncUrl" : <Url of the Zsync Control File>
    }




The *QJsonObject* will follow the following format with respect to json for ```Action::CheckForUpdate``` action,

    {
        "UpdateAvailable": <Boolean, True if update is available>,
        "AbsolutePath" : <Absolute path to local AppImage>,
        "LocalSha1Hash" : <Sha1 Hash of local AppImage>,
        "RemoteSha1Hash" : <Sha1 Hash of Remote AppImage>,
        "ReleaseNotes": <Release notes of the latest release if found>,
        "TorrentSupported": <Boolean, True if torrent update is supported>
    }     




The *QJsonObject* will follow the following format with respect to json for ```Action::Update``` action(And all other update action variants),
    
    {
        "OldVersionPath": <Absolute Path to the old version>,
        "NewVersionPath": <Absolute Path to the new version>,
        "UsedTorrent": <Boolean, True if torrent was used to update
    } 

### error(short, short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](AppImageUpdaterBridgeErrorCodes.html).


### progress(int percentage , qint64 bytesReceived , qint64 bytesTotal , double speed , QString speedUnits, short action)
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
| action         | The action this progress refers to.                              |

### logger(QString , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater issues a log message with the *first QString* as the log message and
the *second QString* as the path to the respective AppImage.

