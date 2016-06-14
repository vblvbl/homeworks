QT       += core
QT       -= core

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SA_TSP
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp \
         ../../qcustomplot.cpp

HEADERS  += mainwindow.h \
         ../../qcustomplot.h

FORMS    += mainwindow.ui

