include(../../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
TEMPLATE = app
CONFIG += release 
TARGET = SimpleUpdateGUI

SOURCES += main.cc
HEADERS += MyUpdateWidget.hpp
