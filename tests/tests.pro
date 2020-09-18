include(../QAppImageUpdate.pri)
INCLUDEPATH += .
CONFIG += release
TARGET = tests
QT += testlib
SOURCES += main.cc
HEADERS += QAppImageUpdateTests.hpp SimpleDownload.hpp
