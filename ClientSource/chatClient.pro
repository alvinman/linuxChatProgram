#-------------------------------------------------
#
# Project created by QtCreator 2016-03-14T18:37:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = chatClient
TEMPLATE = app


SOURCES += main.cpp\
        client.cpp \
    clienthelper.cpp \
    receivethread.cpp

HEADERS  += client.h \
    clienthelper.h \
    receivethread.h

FORMS    += client.ui

RESOURCES += qdarkstyle/style.qrc


