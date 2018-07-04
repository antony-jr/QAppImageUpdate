include(../include/include.pri)
include(../src/src_without_lib.pri)


QT += network

TEMPLATE = app
CONFIG += debug
TARGET = zsync

SOURCES += main.cc
HEADERS += basic_fetcher.hpp
