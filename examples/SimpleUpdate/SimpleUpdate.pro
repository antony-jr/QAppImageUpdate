include(../../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
TEMPLATE = app
TARGET = SimpleUpdate

SOURCES += main.cc \
           cutelog.c
HEADERS += cutelog.h
