QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = campus_navigation_qt
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        mapcanvas.cpp \
        graph.cpp

HEADERS += \
        mainwindow.h \
        mapcanvas.h \
        graph.h

FORMS += \
        mainwindow.ui

RESOURCES += \
        resources.qrc

DISTFILES += \
        campus.map
