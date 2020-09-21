#-------------------------------------------------
#
# Project created by QtCreator 2020-02-22T14:51:55
#
#-------------------------------------------------

QT += core gui
QT += core gui printsupport
QT += core gui axcontainer
QT += widgets
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_usb_video

TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    qextserialbase.cpp \
    qextserialport.cpp \
    win_qextserialport.cpp \
    inputdialog.cpp \
    modifydialog.cpp \
    updatedialog.cpp \
    qcustomplot.cpp \
    getalldialog.cpp

HEADERS += mainwindow.hpp \
    qextserialbase.h \
    qextserialport.h \
    win_qextserialport.h \
    inputdialog.h \
    modifydialog.h \
    updatedialog.h \
    qcustomplot.h \
    getalldialog.h

FORMS += mainwindow.ui\
    inputdialog.ui \
    modifydialog.ui \
    updatedialog.ui \
    getalldialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/opencv2/x64/vc14/lib/ -lopencv_world401
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/opencv2/x64/vc14/lib/ -lopencv_world401d
else:unix: LIBS += -L$$PWD/opencv2/x64/vc14/lib/ -lopencv_world401

INCLUDEPATH += $$PWD/opencv2
DEPENDPATH += $$PWD/opencv2

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lQt5AxBase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lQt5AxBased
else:unix: LIBS += -L$$PWD/./ -lQt5AxBase

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lQt5AxContainer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lQt5AxContainerd
else:unix: LIBS += -L$$PWD/./ -lQt5AxContainer

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

RC_ICONS = appico.ico
