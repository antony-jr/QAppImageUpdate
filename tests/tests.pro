include(../QAppImageUpdate.pri)
INCLUDEPATH += .
CONFIG += release
TARGET = tests
QT += testlib core concurrent
SOURCES += main.cc
HEADERS += QAppImageUpdateTests.hpp SimpleDownload.hpp
