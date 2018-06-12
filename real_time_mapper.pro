#-------------------------------------------------
#
# Project created by QtCreator 2018-06-01T09:59:14
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = real_time_mapper
TEMPLATE = app

INCLUDEPATH += /usr/locale/opencv/include/opencv \
               /usr/locale/opencv/include/opencv2

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    calibrationparameters.cpp \
    stereoimage.cpp \
    readlocalfilethread.cpp \
    modelpainting.cpp \
    libelas/src/descriptor.cpp \
    libelas/src/elas.cpp \
    libviso2/src/filter.cpp \
    libviso2/src/matcher.cpp \
    libviso2/src/matrix.cpp \
    libviso2/src/triangle.cpp \
    libviso2/src/viso.cpp \
    libviso2/src/viso_stereo.cpp \
    reconstructionthread.cpp \
    getcameraposethread.cpp

HEADERS  += mainwindow.h \
    calibrationparameters.h \
    stereoimage.h \
    readlocalfilethread.h \
    modelpainting.h \
    libelas/src/descriptor.h \
    libelas/src/elas.h \
    libviso2/src/filter.h \
    libviso2/src/matcher.h \
    libviso2/src/matrix.h \
    libviso2/src/timer.h \
    libviso2/src/triangle.h \
    libviso2/src/viso.h \
    libviso2/src/viso_stereo.h \
    reconstructionthread.h \
    getcameraposethread.h

FORMS    += mainwindow.ui

LIBS += -lglut -lGLU -L/usr/locale/opencv/lib \
                 -lopencv_core\
                 -lopencv_highgui\
                 -lopencv_imgproc\
                 -lopencv_imgcodecs\
        -L"/usr/lib/x85_64-linux-gnu" -lexpat

QMAKE_CXXFLAGS += -O3 -pipe -fomit-frame-pointer -msse3 -std=c++0x
