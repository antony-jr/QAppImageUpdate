---
id: Installation
title: Installing AppImage Updater Bridge to your Project.
sidebar_label: Installation
---

**AppImage Updater Bridge** is a small library written in C++ using Qt5 Framework. This   
little library helps you to **create awesome AutoUpdater in Qt5 for AppImages**!

**AppImage Updater Bridge** also supports the **Qt** event loop and does not block your main thread and thus this library is a perfect fit for your Qt Projects.

**Before jumping the gun , make sure you have the [dependencies](#dependencies)**

## Dependencies

* [Qt5 Framework](https://qt.io)
	
	Qt is an amazing **GUI Framework** which is made for the future , it got   
	**stability , performance and the looks!**. I really recommend you all to   
	use Qt for your **GUI** Projects. And the best part is , its **Open Source**.   
	Make sure to install it too. **Curl** is also required but since Qt5 will   
	have a copy of curl inside it , You do not need to install it seperately.

## Installing the latest release from github

**Just execute this command on your project folder and everything will be done for you!**   
You must have **git** to do this , **don't worry** because most of the linux distro's must   
have **installed it already** for you , if not install it!

```
 $ git clone --recursive https://github.com/antony-jr/AppImageUpdaterBridge
```

or **to install it in your git project folder**

```
 $ git submodule init
 $ git submodule add https://github.com/antony-jr/AppImageUpdaterBridge
 $ git submodule update --recursive
```

Since **AppImages** are only available to linux , this library is also specific to **Linux**.
