include(../../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
TEMPLATE = app
TARGET = SimpleUpdate

SOURCES += main.cc \
           TextProgressBar.cc
HEADERS += TextProgressBar.hpp
