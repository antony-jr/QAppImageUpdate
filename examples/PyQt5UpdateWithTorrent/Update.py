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

def handleError(code, action):
    print("ERROR: {}".format(obj.errorCodeToDescriptionString(code)))
    app.quit()

def handleFinishedSignal(result, action):
    if action == obj.getConstant("Action::UpdateWithTorrent"):
        for i in result:
            if i == "UsedTorrent":
                used = "No"
                if result[i].toBool():
                    used = "Yes"
                print("{} : {}".format(i, used))
                continue
            print("{} : {}".format(i, result[i].toString()))
        app.quit()
    elif action == obj.getConstant("Action::CheckForUpdate"): 
        if result['UpdateAvailable'].toBool():
            print("A new version of the AppImage is available.")
            print("Updating now... ")
            obj.start(obj.getConstant("Action::UpdateWithTorrent"))
        else:
            print("You have the latest AppImage!")
            app.quit()

obj.finished.connect(handleFinishedSignal)
obj.error.connect(handleError)

obj.setAppImagePath(appimage_path)

print("Checking for Update... ")
obj.start(obj.getConstant("Action::CheckForUpdate"))
sys.exit(app.exec_())
