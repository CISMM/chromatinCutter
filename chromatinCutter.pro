#-------------------------------------------------
#
# Project created by QtCreator 2011-05-18T16:59:54
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = chromatinCutter
TEMPLATE = app

INCLUDEPATH += "C:/Qwt-6.0.0/include"

LIBS += -L"C:/Qwt-6.0.0/lib" -lqwt -lqwtmathml


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    qwt_histogram.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    qwt_histogram.h \
    histogram_values_passer.h

FORMS    += mainwindow.ui
