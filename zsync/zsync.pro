include(../AppImageUpdaterBridge.pri)
TEMPLATE = app
CONFIG += debug
TARGET = zsync

SOURCES += main.cc
HEADERS += ../include/BlockReply_p.hpp \
	   ../include/BlockDownloader_p.hpp
