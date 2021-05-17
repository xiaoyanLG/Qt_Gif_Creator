#-------------------------------------------------
#
# Project created by QtCreator 2019-05-03T17:36:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qt_Gif_Creator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
    xygifcreator.cpp \
    xygifframe.cpp \
    xymovablewidget.cpp \
    xypackimage.cpp

HEADERS += \
    xygifcreator.h \
    xygifframe.h \
    xymovablewidget.h \
    xypackimage.h \
    gif.h

FORMS += \
    xygifframe.ui \
    xypackimage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

win32 {
LIBS += -lGdi32 -lUser32
msvc {
greaterThan(QT_MAJOR_VERSION, 4): RC_FILE = ico.rc
}
} else {
LIBS += -lX11 -lXfixes
}
