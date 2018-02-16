# BSD-3 Clause.
# Copyright (C) 2018 Antony Jr.
# All Rights Reserved.
# ==========================================
# Helps you to build this library statically 
# with Qt static version , Since CMake does
# not go well with Qt Static linking.
# ==========================================
# MUST COMPLY WITH LGPL IF STATICALLY LINKED
# ******************************************
TEMPLATE = lib
CONFIG += staticlib
TARGET = z
INCLUDEPATH += . ..
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += crc32.h  deflate.h  inffixed.h  inflate.h  inftrees.h  trees.h  zconf.h  zlib.h  zutil.h
SOURCES += adler32.c  compress.c  crc32.c  deflate.c  gzio.c  inflate.c  inftrees.c  trees.c  zutil.c
