#-------------------------------------------------
#
# Project created by QtCreator 2018-05-17T17:10:24
#
#-------------------------------------------------

QT += core
QT -= gui

TARGET = cpptool
TEMPLATE = app
DESTDIR = $${OUT_PWD}/bin

CONFIG += console
CONFIG -= app_bundle

#-------------------------------------------------
#      precompile header
#-------------------------------------------------

CONFIG += precompile_header
PRECOMPILED_HEADER = pch.h

#-------------------------------------------------
#      files
#-------------------------------------------------

SOURCES += \
    main.cpp \
    includesorter.cpp \
    enumtodebug.cpp

HEADERS += \
    pch.h \
    includesorter.h \
    enumtodebug.h

FORMS +=
