TEMPLATE = lib
CONFIG += staticlib
TARGET = AIUpdaterBridge
LIBS += ../zlib/libz.a ../librcksum/librcksum.a ../libzsync/libzsync.a -lcurl
QT += core network concurrent
INCLUDEPATH += . .. ../QAIUpdateInformation ../libzsync ../librcksum ../libz
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += AIUpdaterBridge.hpp legacy_http.h ../zsglobal.h
SOURCES += AIUpdaterBridge.cpp legacy_http.c
