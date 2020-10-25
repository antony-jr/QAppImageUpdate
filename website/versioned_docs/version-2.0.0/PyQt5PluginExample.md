---
id: version-2.0.0-PyQt5PluginExample
title: Using QAppImageUpdate Plugin in PyQt5
sidebar_label: Using Plugin in PyQt5
original_id: PyQt5PluginExample
---

This guide Demonstrates how to use the *QAppImageUpdate* plugin to update a single AppImage file.
This example assumes you are using PyQt5 as your python binding to Qt framework.

> Note that if the plugin is placed in the predefined Qt Plugin path, then you don't need the
> absolute path of the plugin. Simply set the file name to 'libQAppImageUpdate'.


## Building the Plugin

```
 $ git clone https://github.com/antony-jr/QAppImageUpdate
 $ cd QAppImageUpdate
 $ mkdir build
 $ cd build 
 $ cmake -DBUILD_AS_PLUGIN ..
 $ make -j$(nproc)
 $ export PLUGIN_PATH=$(pwd)/libQAppImageUpdate.so
```

## Update.py

```
#!/usr/bin/env python3
import os
import sys
from PyQt5.QtCore import QPluginLoader
from PyQt5.QtCore import QCoreApplication

if len(sys.argv) < 2:
    print("Usage: ./Update.py [APPIMAGE PATH]")
    sys.exit(0)

app = QCoreApplication(sys.argv)

# Try to load the plugin from predefined 
# Qt Plugin paths.
loader = QPluginLoader()
loader.setFileName('libQAppImageUpdate')
if not loader.load():
    try:
        plugin_path = os.eviron['PLUGIN_PATH']
    except:
        print("Unable to resolve plugin path.")
        sys.exit(0)
    loader.setFileName(plugin_path)
    if not loader.load():
        print("Cannot load plugin because: {}".format(loader.errorString()))
        sys.exit(-1)


appimage_path = sys.argv[1]
obj = loader.instance()

def handleFinish(info, action):
    if action == obj.getConstant("Action::CheckForUpdate"):
	if info["UpdateAvailable"].toBool():
            print("A new version of the AppImage is available.")
            print("Updating now... ")
            obj.start(obj.getConstant("Action::Update"))
    elif action == obj.getConstant("Action::Update"):
    	print(info)
    	app.quit()
   
def handleError(code):
    print("ERROR: {}".format(obj.errorCodeToDescriptionString(code)))
    app.quit()

obj.finished.connect(handleFinish)
obj.error.connect(handleError)

obj.setAppImagePath(appimage_path)

print("Checking for Update... ")
obj.start(obj.getContant("Action::CheckForUpdate"))
sys.exit(app.exec_())
```

## Execution

```
 $ chmod +x Update.py
 $ ./Update.py some.AppImage
```

See the examples directory in the source tree for more examples.
