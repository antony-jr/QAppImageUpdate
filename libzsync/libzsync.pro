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
TARGET = zsync
INCLUDEPATH += . ..
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += zmap.h zsync.h
SOURCES += zmap.c zsync.c