include(../AppImageUpdaterBridge.pri)
TEMPLATE = app
CONFIG += debug
TARGET = zsync

SOURCES += main.cc \
           textprogressbar.cc
HEADERS += textprogressbar.hpp
