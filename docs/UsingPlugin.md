---
id: UsingPlugin
title: Build QAppImageUpdate as Qt Plugin
sidebar_label: Building as Qt Plugin.
---

QAppImageUpdate can compiled as a **Qt Plugin**. When it is built as a Qt Plugin, QAppImageUpdate can be used from any programming language that supports Qt.

Qt plugins thus makes us to maintain a single code base instead of maintaining a lot of ports. This makes less bug in the code base and there is no time wasted in making bindings.

You can see the plugin interface [here](PluginInterface.html).

To read more about Qt plugins, Please refer the official documentation.

# Rules in Plugin Usage

There are specific rules on how plugins are handled by Qt. The following are the key rules in plugin usage.

* A Qt framework whose major version is higher or equal to the Qt framework 
  used to build the plugin should work fine with no errors. 

* If you build against Qt Framework 5.6.0 then all Qt Framework above and equal to 
  5.6.0 can use the plugin without re-compiling.

* You cannot use the plugin in Qt Framework having lower major version than the 
  Qt Framework used to build the plugin.


# Using QMake.

You just need to enable the ```BUILD_AS_PLUGIN``` flag in your config.
```
  $ qmake "CONFIG+=BUILD_AS_PLUGIN" [ProjectFolder]
```

Now you should have ```libQAppImageUpdate.so``` file which is your plugin. Now use this file
with ```QPluginLoader``` provided by your Qt bindings for your specific programming language.

See the guides on examples on how this is done.

# Using CMake

Same as in QMake, you just need to enable ```BUILD_AS_PLUGIN``` flag,

```
 $ cmake -DBUILD_AS_PLUGIN=ON [ProjectFolder]
```

Now you should have ```libQAppImageUpdate.so``` file which is your plugin. Now use this file
with ```QPluginLoader``` provided by your Qt bindings for your specific programming language.

See the guides on examples on how this is done.
