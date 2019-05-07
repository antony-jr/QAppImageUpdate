---
id: ClassAppImageDeltaRevisioner
title: Class AppImageDeltaRevisioner
sidebar_label: Class AppImageDeltaRevisioner
---

|	    |	        	                                       |		
|-----------|----------------------------------------------------------|
|  Header:  | #include < AppImageUpdaterBridge >                         |
|   qmake:  | include(AppImageUpdaterBridge/AppImageUpdaterBridge.pri) |
|Inherits:  | [QObject](http://doc.qt.io/qt-5/qobject.html)            |
|Namespace: | **AppImageUpdaterBridge**


> **Important**: AppImageDeltaRevisioner is under AppImageUpdaterBridge namespace , Make sure to include it.


AppImageDeltaRevisioner is the main class which *Reads Embeded Update Information* , *Checks for Updates* , *Writes the Delta* ,
*Downloads The Remaining Blocks* and  finally *Verifies the Checksums.*

In each stage , You can connect with specific signals to listen to each stage of the revision process(You can also control them).
Thus all you need is this class to do the entire update.

Internally this class purely uses **signals and slots** to communicate to private classes which are single thread and non-blocking(Optionally runs in a seperate thread). This class only holds a pointer to the private implementation (*PIMPL*).
For most of the time , This class does not leak memory but if it does please report it on github.

All methods in this class is [reentrant](https://doc.qt.io/qt-5/threads-reentrancy.html) and thread safe.

Eventhough all methods are reentrant , This class does not use **mutex** thanks to **Qt's signals and slots everything is cyclic and thus no mutex is needed.**

## Public Functions

| Return Type  | Name |
|--------------|------------------------------------------------------------------------------------------------|
|  | [AppImageDeltaRevisioner(bool singleThreaded = true, QObject \*parent = nullptr)](#appimagedeltarevisionerbool-singlethreaded-true-qobject-parent-nullptr) |
|  | [AppImageDeltaRevisioner(const QString&, bool singleThreaded = true, QObject \*parent = nullptr)](#appimagedeltarevisionerconst-qstring-bool-singlethreaded-true-qobject-parent-nullptr) |
|  | [AppImageDeltaRevisioner(QFile \*, bool singleThreaded = true, QObject \*parent = nullptr)](#appimagedeltarevisionerqfile-bool-singlethreaded-true-qobject-parent-nullptr) |


## Slots

| Return Type  | Name |
|------------------------------|-------------------------------------------|
| **void** | [start(void)](#void-startvoid) |
| **void** | [cancel(void)](#void-cancelvoid) |
| **void** | [setAppImage(const QString&)](#void-setappimageconst-qstring) |
| **void** | [setAppImage(QFile \*)](#void-setappimageqfile) |
| **void** | [setShowLog(bool)](#void-setshowlogbool) |
| **void** | [setOutputDirectory(const QString&)](#void-setoutputdirectoryconst-qstring) |
| **void** | [setProxy(const QNetworkProxy&)](#void-setproxyconst-qnetworkproxy-https-docqtio-qt-5-qnetworkproxyhtml) |
| **void** | [getAppImageEmbededInformation(void)](#void-getappimageembededinformationvoid) |
| **void** | [checkForUpdate(void)](#void-checkforupdatevoid) |
| **void** | [clear(void)](#void-clearvoid) |

## Signals

| Return Type  | Name |
|--------------|------------------------------------------------|
| void | [started(void)](#void-startedvoid) |
| void | [canceled(void)](#void-canceledvoid) |
| void | [finished(QJsonObject , QString)](#void-finishedqjsonobject-qstring) |
| void | [embededInformation(QJsonObject)](#void-embededinformationqjsonobject) |
| void | [updateAvailable(bool, QJsonObject)](#void-updateavailablebool-qjsonobject) |
| void | [statusChanged(short)](#void-statuschangedshort) |
| void | [error(short)](#void-errorshort) |
| void | [progress(int, qint64, qint64, double, QString)](#void-progressint-percentage-qint64-bytesreceived-qint64-bytestotal-double-speed-qstring-speedunits) |
| void | [logger(QString, QString)](#void-loggerqstring-qstring) |


## Member Functions Documentation

### AppImageDeltaRevisioner(bool singleThreaded = true, QObject \*parent = nullptr)

Default Constructor , Constructs the Updater with a assumed AppImage Path which is most likely the 
AppImage which is running this updater. The assumed AppImage Path will only be correct if and only if
this updater is running from a AppImage.

The default value for **singleThreaded** is **true** but you can set it to **false** to run all the 
resource of the updater in a seperate thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**

```
using namespace AppImageUpdaterBridge;
AppImageDeltaRevisioner DRevisioner;
```

### AppImageDeltaRevisioner(const QString&, bool singleThreaded = true, QObject \*parent = nullptr)

This is an overloaded constructor , Constructs the Updater with the given QString as the AppImage Path.
If the given QString is not a valid AppImage Path, Then the updater will automatically
guess the AppImage Path , The guessed AppImage Path will be accurate only if the updater is running from 
an AppImage.

The default value for **singleThreaded** is **true** but you can set it to **false** to run all the 
resource of the updater in a seperate thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**

### AppImageDeltaRevisioner(QFile \*, bool singleThreaded = true, QObject \*parent = nullptr)

This is an overloaded constructor , Constructs the Updater with the given QFile as the AppImage itself.
If the pointer is invalid or has other sort of read errors , The updater will emit error but when 
forced to start , Again the updater guesses the AppImage Path in order to continue with the extraction.
If the guessed AppImage Path is not an AppImage but a normal elf file then this result in a invalid magic
byte error. See [error codes](https://antony-jr.github.io/AppImageUpdaterBridge/docs/AppImageDeltaRevisionerErrorCodes.html) for more information.

The default value for **singleThreaded** is **true** but you can set it to **false** to run all the 
resource of the updater in a seperate thread excluding **this class**.

You can set a **QObject parent** to make use of **Qt's Parent to Children deallocation.**

### void start(void)
<p align="right"> <b>[SLOT]</b> </p>

Starts the updater.
Emits **started()** signal when starts.


> Minor Note: You don't have to worry about anything if you called checkForUpdate or getAppImageEmbededInformation 
slots before start , Don't worry about overheads too , Since when you call checkForUpdate slot , The information
is cached and when start slot is called again , it will be faster than normal. 

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


### void getAppImageEmbededInformation(void)
<p align="right"> <b>[SLOT]</b> </p>

Requests the updater for the embeded information of the current operating AppImage.
Emits ** embededInformation(QJsonObject) ** in which the *QJsonObject* will be the embeded information 
of the AppImage in the below format with respect to Json.

    { "IsEmpty" : false ,
      "FileInformation" :   { "AppImageFilePath" : The given AppImage Path ,
                              "AppImageSHA1Hash" : The given AppImage's SHA1 Hash ,
                            },
      "UpdateInformation":  {
                              "transport" : gh-releases-zsync / bintray-zsync / zsync,
                               Depending on the transport the values down below will varry.
                            }
    }

**gh-releases-zsync** , 

    {
    "transport" : "gh-releases-zsync",
    "username"  : Username of the author ,
     "repo"     : Name of the repo ,
     "tag"      : continuous / latest / other,
     "filename" : Latest AppImage filename with wild card
    }

**bintray-zsync** , 

    {
        "transport" : "bintray-zsync" ,
        "username"  :  Username of the author ,
        "repo"      :  Name of the repo ,
        "packageName" : Name of the package in bintray ,
        "filename" : Name of the latest AppImage fileanme
    }

**zsync**,

    {
        "transport" : "zsync",
        "zsyncUrl" : Url of the Zsync Control File
    }


> Important: If the AppImage is not given then calling this method will guess the AppImage 
Path and so if the returned AppImage is not a valid , Then it emits an error signal.


### void checkForUpdate(void)
<p align="right"> <b>[SLOT]</b> </p>

Checks update for the current operating AppImage.
emits **updateAvailable(bool , QJsonObject)** , Where the *bool*  will be **true** if the AppImage
needs update. The QJsonObject in the signal will have the details of the current operating
AppImage.


### void clear(void)
<p align="right"> <b>[SLOT]</b> </p>

Clears all internal **cache**.


### void started(void)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is started successfully.

### void canceled(void)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is canceled successfully.

### void finished(QJsonObject , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the update is finished successfully. The given *QJsonObject* has the details of the new version
of the AppImage and the given *QString* has the absolute path to the old versioin of the AppImage.

The *QJsonObject* will follow the folloing format with respect to json ,
	
    {
        "AbsolutePath" : Absolute path of the new version of the AppImage ,
        "Sha1Hash"     : Sha1 hash of the new version of the AppImage
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
        "AbsolutePath" : The absolute path of the current operating AppImage ,
        "Sha1Hash"     : The Sha1 hash of the current operating AppImage
    }

### void statusChanged(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater status is changed , The given short integer is the status code.
See [status codes](https://antony-jr.github.io/AppImageUpdaterBridge/docs/AppImageUpdaterBridgeStatusCodes.html).


### void error(short)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater is errored. The given short integer is the error code.
See [error codes](https://antony-jr.github.io/AppImageUpdaterBridge/docs/AppImageUpdaterBridgeErrorCodes.html).


### void progress(int percentage , qint64 bytesReceived , qint64 bytesTotal , double speed , QString speedUnits)
<p align="right"> <b>[SIGNAL]</b> </p>

The updater's progress is emitted through this unified signal.

**Where** ,

    'percentage' is the percentage finished revising the latest AppImage.
    'bytesReceived' is the received bytes of the latest AppImage.
    'bytesTotal' is the total bytes of the latest AppImage.
    'speed' is the transfer speed value.
    'speedUnits' is the transfer speed unit(e.g. KiB/s , etc... ) for 'speed'.


### void logger(QString , QString)
<p align="right"> <b>[SIGNAL]</b> </p>

Emitted when the updater issues a log message with the *first QString* as the log message and
the *second QString* as the path to the respective AppImage.


