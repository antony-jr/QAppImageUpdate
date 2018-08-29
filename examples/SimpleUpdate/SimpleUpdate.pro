include(../../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
TEMPLATE = app
CONFIG += release without_widgets 
TARGET = SimpleUpdate

SOURCES += main.cc \
           TextProgressBar.cc
HEADERS += TextProgressBar.hpp
