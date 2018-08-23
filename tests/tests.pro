include(../AppImageUpdaterBridge.pri)
INCLUDEPATH += .
CONFIG += release
QT += testlib
SOURCES += main.cc
HEADERS += AppImageUpdateInformation.hpp \
	   ZsyncRemoteControlFileParser.hpp
