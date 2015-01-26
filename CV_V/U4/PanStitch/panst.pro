TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ./src/saves.cpp \
    ./src/render.cpp \
    ./src/ps_main.cpp \
    ./src/mean.cpp \
    ./src/matching.cpp \
    ./src/local_maxima.cpp \
    ./src/homographies.cpp \
    ./src/harris.cpp \
    src/matching_marriage.cpp

HEADERS += ./src/ps.h

win32 {
	INCLUDEPATH += C:\opencv\build\include
	debug: LIBS += -LC:\opencv\build\lib\Debug -lopencv_core247d -lopencv_highgui247d -lopencv_imgproc247d -lopencv_calib3d247d
	release: LIBS += -LC:\opencv\build\lib\Release -lopencv_core247 -lopencv_highgui247 -lopencv_imgproc247 -lopencv_calib3d247
}

unix: LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d