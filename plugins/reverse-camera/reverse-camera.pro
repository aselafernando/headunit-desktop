TEMPLATE = lib
CONFIG += plugin link_pkgconfig
QT += quick widgets
PKGCONFIG = gstreamer-1.0 gstreamer-video-1.0

TARGET = $$qtLibraryTarget(reverse-camera)

include("../../config.pri")
target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    reverse-camera.cpp

HEADERS += \
    reverse-camera.h

RESOURCES += reverse-camera.qrc

DISTFILES += \
    config.json
