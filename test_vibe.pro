QT += core
QT -= gui

CONFIG += c++11

TARGET = test_vibe
CONFIG += console
CONFIG -= app_bundle

#DEFINES+= \
#    YUYV \
#    RECORD_FUNC \
#    DUMP_H264 \
#    GRAYSCALE_FORE_DETECT \
#    LOG_INTRUDE_DEBUG \
#    LOG_SCENE_DEBUG \
#    OPENCV_DISPLAY \

TEMPLATE = app

HEADERS += \
    H264Encoder.h \
    encoder_define.h \
    UVCCamera.h \
    Utils.h \
    VideoRecorder.h \
    SceneDetector.h \
    IDP.h \
    IntrudeDetector.h \
    ForegroundDetect.h \
    RingBuffer.h \
    Macro.h \
    tiny_jpeg.h \
    Convert.h

SOURCES += \
#    main-opencv.cpp \
    H264Encoder.cpp \
    UVCCamera.cpp \
    VideoRecorder.cpp \
    main.cpp \
    SceneDetector.cpp \
    IDP.cpp \
    IntrudeDetector.cpp \
    RingBuffer.cpp \
    ForegroundDetect.cpp \
    Convert.cpp

INCLUDEPATH += \
#opencv2
/usr/local/opencv2.4.13/include \
/usr/local/opencv2.4.13/include/opencv \
#opencv3
#/usr/local/opencv320/include \
#/usr/local/opencv320/include/opencv \
#/usr/local/opencv320/include/opencv2 \
#/usr/local/opencv320/include/opencv2/flann \

LIBS += \
#-lboost_system \
#opencv2
/usr/local/opencv2.4.13/lib/libopencv_*.so \
/usr/local/opencv2.4.13/lib/libopencv_highgui.so.2.4 \
#h264
/usr/lib/libx264.so \
