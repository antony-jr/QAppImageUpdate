---
id: AppImageDeltaRevisionerExample
title: AppImageDeltaRevisioner Simple Update Example.
sidebar_label: AppImageDeltaRevisioner Simple Update Example
---

Demonstrates how to use the *AppImageUpdaterBridge* APIs for updating a single AppImage file.
The Simple Update Example parses the path from the program arguments , And uses the *[AppImageDeltaRevisioner]()* class to
perform the actula delta update.

The Simple Update Example also handle errors and shows text based progress bar.

> Important: Please make sure to modify the 'include([AppImage Updater Bridge Path]/AppImageUpdaterBridge.pri)' according 
  to the AppImageUpdaterBridge source path.


## Files: 

* [/examples/SimpleUpdate/main.cc](https://raw.githubusercontent.com/antony-jr/AppImageUpdaterBridge/master/examples/SimpleUpdate/main.cc)
* [/examples/SimpleUpdate/TextProgressBar.cc](https://raw.githubusercontent.com/antony-jr/AppImageUpdaterBridge/master/examples/SimpleUpdate/TextProgressBar.cc)
* [/examples/SimpleUpdate/TextProgressBar.hpp](https://raw.githubusercontent.com/antony-jr/AppImageUpdaterBridge/master/examples/SimpleUpdate/TextProgressBar.hpp)
* [/examples/SimpleUpdate/SimpleUpdate.pro](https://raw.githubusercontent.com/antony-jr/AppImageUpdaterBridge/master/examples/SimpleUpdate/SimpleUpdate.pro)


