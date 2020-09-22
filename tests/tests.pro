include(../QAppImageUpdate.pri)
INCLUDEPATH += .
CONFIG += release
TARGET = tests
QT += testlib concurrent
SOURCES += main.cc
HEADERS += QAppImageUpdateTests.hpp SimpleDownload.hpp
