---
id: version-1.1.9-Installation
title: Installing AppImage Updater Bridge to your Project.
sidebar_label: Installation
original_id: Installation
---

Since AppImage Updater Birdge is specifically made for Qt , All you need is Qt's development tools
and only that, Nothing more and nothing less.

## Dependencies

* [Qt5 Framework](https://qt.io)


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

Even though AppImage is specific to linux , this library is cross platform.
