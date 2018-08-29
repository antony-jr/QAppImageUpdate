include(../../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
TEMPLATE = app
CONFIG += release 
TARGET = SimpleUpdate

SOURCES += main.cc
HEADERS += MyUpdateWidget.hpp
