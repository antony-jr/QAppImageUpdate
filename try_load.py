#!/usr/bin/env python3
import os
import sys
from PyQt5.QtCore import QPluginLoader
from PyQt5.QtCore import QCoreApplication

app = QCoreApplication(sys.argv)

# Try to load the plugin from predefined 
# Qt Plugin paths.
loader = QPluginLoader()
loader.setFileName("{}/libQAppImageUpdate.so".format(os.path.realpath('.')))

if loader.load():
    print("Loaded")
else:
    print(loader.errorString())
 
