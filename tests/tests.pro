include(../QAppImageUpdate.pri)
INCLUDEPATH += .
CONFIG += release
TARGET = tests
QT += testlib concurrent
SOURCES += main.cc
HEADERS += QAppImageUpdateTests.hpp SimpleDownload.hpp

QUICK_TEST {
	DEFINES += QUICK_TEST
}
