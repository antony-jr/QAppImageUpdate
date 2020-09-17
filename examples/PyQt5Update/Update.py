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
        plugin_path = os.environ['PLUGIN_PATH']
    except:
        print("Unable to resolve plugin path.")
        sys.exit(0)
    loader.setFileName(plugin_path)
    if not loader.load():
        print("Cannot load plugin because: {}".format(loader.errorString()))
        sys.exit(-1)


appimage_path = sys.argv[1]
obj = loader.instance()

def handleFinish(info, old_appimge_path):
    print(info)
    app.quit()

def handleError(code, action):
    print("A error occured, error code: {}".format(code))
    app.quit()

def handleFinishedSignal(result, action):
    if action == 3:
        for i in result:
            print("{} : {}".format(i, result[i].toString()))
        app.quit()
    elif action == 2: 
        if result['UpdateAvailable']:
            print("A new version of the AppImage is available.")
            print("Updating now... ")
            obj.start(3)
        else:
            print("You have the latest AppImage!")
            app.quit()

obj.finished.connect(handleFinishedSignal)
obj.error.connect(handleError)

obj.setAppImagePath(appimage_path)

print("Checking for Update... ")
obj.start(2)
sys.exit(app.exec_())
