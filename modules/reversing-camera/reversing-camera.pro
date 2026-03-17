TEMPLATE = lib
CONFIG += plugin link_pkgconfig
QT += quick widgets
PKGCONFIG = gstreamer-1.0 gstreamer-video-1.0

TARGET = $$qtLibraryTarget(reversing-camera)

DEFINES += GST_USE_UNSTABLE_API

include("../../config.pri")
target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    reversing-camera.cpp

HEADERS += \
    reversing-camera.h

RESOURCES += qml.qrc

DISTFILES += \
    config.json




