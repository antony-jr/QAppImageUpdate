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
|  | [QAppImageUpdate(bool singleThreaded = true, QObject \*parent = nullptr)](#) |
|  | [QAppImageUpdate(const QString&, bool singleThreaded = true, QObject \*parent = nullptr)](#) |
|  | [QAppImageUpdate(QFile \*, bool singleThreaded = true, QObject \*parent = nullptr)](#) |


## Slots

| Return Type  | Name |
|------------------------------|-------------------------------------------|
| **void** | [start(short)](#) |
| **void** | [cancel()](#) |
| **void** | [setAppImage(const QString&)](#) |
| **void** | [setAppImage(QFile \*)](#) |
| **void** | [setShowLog(bool)](#) |
| **void** | [setOutputDirectory(const QString&)](#) |
| **void** | [setProxy(const QNetworkProxy&)](#void-setproxyconst-qnetworkproxy-https-docqtio-qt-5-qnetworkproxyhtml) |
| **void** | [setGuiFlag(int)]() |
| **void** | [setIcon(QByteArray)]() | 
| **void** | [clear()](#) |

## Signals

| Return Type  | Name |
|--------------|------------------------------------------------|
| void | [started(short)](#) |
| void | [canceled(short)](#) |
| void | [finished(QJsonObject , short)](#) |
| void | [error(short, short)](#) |
| void | [progress(int, qint64, qint64, double, QString, short)](#) |
| void | [logger(QString, QString)](#void-loggerqstring-qstring) |


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

### void start(short action)
<p align="right"> <code>[SLOT]</code> </p>

Starts a specific action as given in the argument of the start slot. 
Valid actions are put up in the Actions table.

### void cancel(void)
<p align="right"> <b>[SLOT]</b> </p>

Cancels the update.
Emits **canceled()** signal when cancel was successfull.


### void setAppImage(const QString&)
<p align="right"> <b>[SLOT]</b> </p>

Sets the AppImage Path as the given **QString**.


### void setAppImage(QFile \*)
<p align="right"> <b>[SLOT]</b> </p>

Sets the given **QFile** as the AppImage itself.

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


### void setProxy(const [QNetworkProxy](https://doc.qt.io/qt-5/qnetworkproxy.html)&)
<p align="right"> <b>[SLOT]</b> </p>

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


### void clear()
<p align="right"> <b>[SLOT]</b> </p>

Clears all internal **cache**.


### void started(short action)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is started successfully.

### void canceled(short action)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is canceled successfully.

### void finished(QJsonObject info, short action)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when a action is finished successfully. The given *QJsonObject* has the details of the new version
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


### void embededInformation(QJsonObject)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when *[getAppImageEmbededInformation(void)](#void-getappimageembededinformationvoid)* is called.

### void updateAvailable(bool , QJsonObject)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when *[checkForUpdate(void)](#void-checkforupdatevoid)* is called.
The given *bool* states if the operating AppImage needs update and the *QJsonObject* gives the details of 
the current operating AppImage.

The *QJsonObject* will follow the following format with respect to json , 
	
    {
        "AbsolutePath" : "The absolute path of the current operating AppImage" ,
        "Sha1Hash"     : "The Sha1 hash of the current operating AppImage" ,
        "RemoteSha1Hash" : "The Sha1 hash of the lastest AppImage" ,
        "ReleaseNotes" : "Release notes if available"
    }

### void statusChanged(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater status is changed , The given short integer is the status code.
See [status codes](AppImageUpdaterBridgeStatusCodes.html).


### void error(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](AppImageUpdaterBridgeErrorCodes.html).


### void progress(int percentage , qint64 bytesReceived , qint64 bytesTotal , double speed , QString speedUnits)
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


### void logger(QString , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater issues a log message with the *first QString* as the log message and
the *second QString* as the path to the respective AppImage.


