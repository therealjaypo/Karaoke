#-------------------------------------------------
#
# Project created by QtCreator 2015-04-18T10:51:35
#
#-------------------------------------------------

QT       += core gui multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Karaoke
TEMPLATE = app


SOURCES += main.cpp\
        karaoke.cpp \
    widgets/cdg.cpp \
    util/musiclibrary.cpp

HEADERS  += karaoke.h \
    widgets/cdg.h \
    util/musiclibrary.h

FORMS    += karaoke.ui \
    widgets/cdg.ui

RESOURCES += \
    Assets.qrc
