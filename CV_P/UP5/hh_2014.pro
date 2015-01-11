#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T16:16:21
#
#-------------------------------------------------

QT       += core

QT       -= gui
#QMAKE_CXXFLAGS += -O3
TARGET = hh_2014
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    markov.cpp

 LIBS += -L/opt/opencv-2.4.9/release/lib
unix:!macx: LIBS += -lopencv_core -lopencv_calib3d -lopencv_contrib -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts
