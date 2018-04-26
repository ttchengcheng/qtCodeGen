QT += core
QT -= gui

CONFIG += c++11

TARGET = qtcodegen
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    enumtodebug.cpp

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    enumtodebug.h
