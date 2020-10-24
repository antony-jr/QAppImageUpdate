---
id: ClassQAppImageUpdate
title: Class QAppImageUpdate
sidebar_label: Class QAppImageUpdate
---

|	    |	        	                                       |		
|-----------|----------------------------------------------------------|
|  Header:  | #include < QAppImageUpdate >                             |
|   qmake:  | include(QAppImageUpdate/QAppImageUpdate.pri)             |
|Inherits:  | [QObject](http://doc.qt.io/qt-5/qobject.html)            |


QAppImageUpdate is the main class which *Reads Embeded Update Information*, *Checks for Updates*, *Writes the Delta*,
*Downloads The Remaining Blocks* and  finally *Verifies the Checksums.*

In each stage, You can connect with specific signals to listen to each stage of the revision process(You can also control them).
Thus all you need is this class to do the entire update.

Internally this class purely uses **signals and slots** to communicate to private classes which are single thread and non-blocking(Optionally runs in a separate thread). This class only holds a pointer to the private implementation (*PIMPL*).
For most of the time, This class does not leak memory but if it does please report it on github.

All methods in this class is [reentrant](https://doc.qt.io/qt-5/threads-reentrancy.html) and thread safe.

## Public Functions

| Return Type  | Name |
|--------------|------------------------------------------------------------------------------------------------|
|  | [QAppImageUpdate(bool singleThreaded = true, QObject \*parent = nullptr)](#qappimageupdatebool-singlethreaded--true-qobject-parent--nullptr) |
|  | [QAppImageUpdate(const QString&, bool singleThreaded = true, QObject \*parent = nullptr)](#qappimageupdateconst-qstring-bool-singlethreaded--true-qobject-parent--nullptr) |
|  | [QAppImageUpdate(QFile \*, bool singleThreaded = true, QObject \*parent = nullptr)](#qappimageupdateqfile--bool-singlethreaded--true-qobject-parent--nullptr) |


## Slots

| Return Type  | Name |
|------------------------------|-------------------------------------------|
| **void** | [setGuiFlag(int)](#void-setguiflagint-flag) |
| **void** | [setIcon(QByteArray)](#void-seticonqbytearray-icon) |
| **void** | [start(short)](#void-startshort-action) |
| **void** | [cancel()](#void-cancel) |
| **void** | [setAppImage(const QString&)](#void-setappimageconst-qstring) |
| **void** | [setAppImage(QFile \*)](#void-setappimageqfile-) |
| **void** | [setShowLog(bool)](#void-setshowlogbool) |
| **void** | [setOutputDirectory(const QString&)](#void-setoutputdirectoryconst-qstring) |
| **void** | [setProxy(const QNetworkProxy&)](#void-setproxyconst-qnetworkproxyhttpsdocqtioqt-5qnetworkproxyhtml) |
| **void** | [clear()](#void-clear) |

## Signals

| Return Type  | Name |
|--------------|------------------------------------------------|
| void | [started(short)](#void-startedshort-action) |
| void | [canceled(short)](#void-canceledshort-action) |
| void | [finished(QJsonObject , short)](#void-finishedqjsonobject-info-short-action) |
| void | [error(short, short)](#void-errorshort-errorcode-short-action) |
| void | [progress(int, qint64, qint64, double, QString, short)](#void-progressint-percentage--qint64-bytesreceived--qint64-bytestotal--double-speed--qstring-speedunits-short-action) |
| void | [logger(QString, QString)](#void-loggerqstring--qstring) |


## Static Public Members

| Return Type  | Name                                                 |
|--------------|------------------------------------------------------|
| QString      | [errorCodeToString(short)](#qstring-errorcodetostringshort-errorcode)|
| QString      | [errorCodeToDescriptionString(short)](#qstring-errorcodetodescriptionstringshort-errorcode) |



## Actions

| Variable Name                                    | Value |
|--------------------------------------------------|------ |
| QAppImageUpdate::Action::GetEmbeddedInfo         |   0   |
| QAppImageUpdate::Action::CheckForUpdate          |   1   |
| QAppImageUpdate::Action::Update                  |   2   |
| QAppImageUpdate::Action::UpdateWithTorrent       |   3   |
| QAppImageUpdate::Action::UpdateWithGUI           |   4   |
| QAppImageUpdate::Action::UpdateWithGUIAndTorrent |   5   |



```
   QAppImageUpdate updater;
   updater.start(QAppImageUpdate::Action::CheckForUpdate); // Checks for update.
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



> The Default flag enables ShowProgressDialog, ShowBeforeProgress, ShowUpdateConfirmationDialog, ShowFinishedDialog,
> ShowErrorDialog, NotifyWhenNoUpdateIsAvailable, NoRemindMeLaterButton, NoSkipThisVersionButton




## Member Functions Documentation

### QAppImageUpdate(bool singleThreaded = true, QObject \*parent = nullptr)

Default Constructor, Constructs the Updater with a assumed AppImage Path 
which is most likely the AppImage which is running this updater. 
The assumed AppImage Path will only be correct if and only if 
this updater is running from a AppImage.

The default value for **singleThreaded** is **true** but you can set it 
to **false** to run all the  resource of the updater in a seperate 
thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**

```
QAppImageUpdate updater;
```

### QAppImageUpdate(const QString&, bool singleThreaded = true, QObject \*parent = nullptr)

This is an overloaded constructor, Constructs the Updater with the given QString as the AppImage Path.
If the given QString is not a valid AppImage Path, Then the updater will automatically
guess the AppImage Path, The guessed AppImage Path will be accurate only if the updater is running from 
an AppImage.

The default value for **singleThreaded** is **true** but you can set it to **false** to run all the 
resource of the updater in a seperate thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**

### QAppImageUpdate(QFile \*, bool singleThreaded = true, QObject \*parent = nullptr)

This is an overloaded constructor, Constructs the Updater with the given QFile as the AppImage itself.
If the pointer is invalid or has other sort of read errors, The updater will emit error but when 
forced to start, Again the updater guesses the AppImage Path in order to continue with the extraction.
If the guessed AppImage Path is not an AppImage but a normal elf file then this result in a invalid magic
byte error. See [error codes](ErrorCodes.html) for more information.

The default value for **singleThreaded** is **true** but you can set it to **false** to run all the 
resource of the updater in a seperate thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**


### void setGuiFlag(int flag)

<p align="right"> <code>[SLOT]</code> </p>

Set the given integer as the flag for GUI update if used. See the [GUI Flags](#gui-flags).

### void setIcon(QByteArray icon)
<p align="right"> <code>[SLOT]</code> </p>

Set the given icon which is saved as QByteArray. You have to save your ```QPixmap``` as a QByteArray, see [the official docs](https://doc.qt.io/qt-5/qpixmap.html#save-1).

```
 /// C++ Ref for saving QPixmap as QByteArray
 QPixmap pixmap;
 QByteArray icon;
 QBuffer buffer(&icon);
 buffer.open(QIODevice::WriteOnly);
 pixmap.save(&buffer, "PNG"); // writes pixmap into bytes in PNG format

 
 QAppImageUpdater updater;
 updater.setIcon(icon);
```

### void start(short action)
<p align="right"> <code>[SLOT]</code> </p>

Starts a specific action as given in the argument of the start slot. 
Valid actions are put up in the [Actions table](#actions).

```
QAppImageUpdater updater;
updater.start(QAppImageUpdate::Action::CheckForUpdate);
```

### void cancel()
<p align="right"> <code>[SLOT]</code> </p>

Cancels the update.
Emits **canceled(short action)** signal when cancel was successfull.


### void setAppImage(const QString&)
<p align="right"> <code>[SLOT]</code> </p>

Sets the AppImage Path as the given **QString**.

> WARNING: If you set the AppImage yourself, Then any guessing will not be done and all
> errors relating to find the correct path to AppImage should be handled by the programmer.
> This is seen in the case of AppImageLauncher where the actuall AppImage can be guessed 
> by the Updater if do not set the path yourself.


### void setAppImage(QFile \*)
<p align="right"> <code>[SLOT]</code> </p>

Sets the given **QFile** as the AppImage itself.

### void setShowLog(bool)
<p align="right"> <code>[SLOT]</code> </p>

Turns on and off the log printer.

> Note: logger signal will be emitted all the time if the library is compiled with LOGGING_DISABLED undefined ,
setShowLog will not affect this activity at all, But setShowLog will print these log messages
if set to true.

### void setOutputDirectory(const QString&)
<p align="right"> <code>[SLOT]</code> </p>

Writes the new version of the AppImage to the given Output directory, Assuming the given QString a directory path.
The default is the old version AppImage's directory.


### void setProxy(const [QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html)&)
<p align="right"> <code>[SLOT]</code> </p>

Sets the given [QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html) as the proxy
to use for all network communication for the updater.

```
   QNetworkProxy proxy;
   proxy.setType(QNetworkProxy::Socks5Proxy);
   proxy.setHostName("127.0.0.1");
   proxy.setPort(9050);

   AppImageDeltaRevisioner Revisioner("Ein.AppImage");
   Revisioner.setProxy(proxy);
   Revisioner.start(); /* Start the updater */
```


> WARNING: when using torrent support, only HTTP and SOCKS5 proxy is supported.


### void clear()
<p align="right"> <code>[SLOT]</code> </p>

Clears all internal **cache**.


### void started(short action)
<p align="right"> <code>[SIGNAL]</code> </p>

Emitted when a action is started successfully.

### void canceled(short action)
<p align="right"> <code>[SIGNAL]</code> </p>

Emitted when the update is canceled successfully.

### void finished(QJsonObject info, short action)
<p align="right"> <code>[SIGNAL]</code> </p>

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



### void error(short errorCode, short action)
<p align="right"> <code>[SIGNAL]</code> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](ErrorCodes.html).


### void progress(int percentage , qint64 bytesReceived , qint64 bytesTotal , double speed , QString speedUnits, short action)
<p align="right"> <code>[SIGNAL]</code> </p>

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


### void logger(QString , QString)
<p align="right"> <code>[SIGNAL]</code> </p>

Emitted when the updater issues a log message with the *first QString* as the log message and
the *second QString* as the path to the respective AppImage.


## QString errorCodeToString(short errorCode)
<p align="right"> <code>[STATIC]</code> </p>

Returns the error as a string denoting the error code.

### QString errorCodeToDescriptionString(short errorCode)
<p align="right"> <code>[STATIC]</code> </p>

Returns a human readable error string for the given error code.
