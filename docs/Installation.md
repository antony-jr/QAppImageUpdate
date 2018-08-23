---
id: Installation
title: Installing AppImage Updater Bridge to your Project.
sidebar_label: Installation
---

Since AppImage Updater Birdge is specifically made for Qt , All you need is Qt's development tools
and only that, Nothing more and nothing less.

## Dependencies

* [Qt5 Framework](https://qt.io)
	
	Qt is an amazing **GUI Framework** which is made for the future , it got   
	**stability , performance and the looks!**. I really recommend you all to   
	use Qt for your **GUI** Projects. And the best part is , its **Open Source**.   
	Make sure to install it too.


## Installing the latest release from github

**Just execute this command on your project folder and everything will be done for you!**   
You must have **git** to do this , **don't worry** because most of the linux distro's must   
have **installed it already** for you , if not install it!

```
 $ git clone https://github.com/antony-jr/AppImageUpdaterBridge
```

or **to install it in your git project folder**

```
 $ git submodule init
 $ git submodule add https://github.com/antony-jr/AppImageUpdaterBridge
 $ git submodule update
```

Since **AppImages** are only available to linux , this library is also specific to **Linux**.
But Qt is cross platform and therefore , This may even run on all platforms where Qt is supported.
