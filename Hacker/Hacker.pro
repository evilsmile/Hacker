#-------------------------------------------------
#
# Project created by QtCreator 2014-07-17T15:32:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Hacker
TEMPLATE = app

QT += network

SOURCES += main.cpp\
        mainwindow.cpp \
    icmpsocket.cpp \
    ipresolver.cpp \
    arpsender.cpp \
    hacker.cpp

HEADERS  += mainwindow.h \
    icmpsocket.h \
    ipresolver.h \
    arpsender.h \
    hacker.h

FORMS    += mainwindow.ui
