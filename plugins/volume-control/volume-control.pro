TEMPLATE = lib
CONFIG += c++17 link_pkgconfig plugin
QT += quick dbus

PKGCONFIG += libpulse-simple libpulse libpulse-mainloop-glib
PKGCONFIG += KF6PulseAudioQt

TARGET = $$qtLibraryTarget(volume-control-plugin)
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $${PWD}/../../includes
DESTDIR = $${OUT_PWD}/../../plugins

include("../../config.pri")

target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    $$PWD/volumecontrol.cpp \
    debug.cpp

HEADERS += \
    $$PWD/volumecontrol.h \
    debug.h

QML_IMPORT_PATH += $$OUT_PWD

RESOURCES += \
    $$PWD/volume-control.qrc

DISTFILES += \
    config.json
