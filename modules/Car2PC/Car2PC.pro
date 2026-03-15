TEMPLATE = lib
CONFIG += c++11 plugin link_pkgconfig
QT += quick serialport
TARGET = $$qtLibraryTarget(car2pc-plugin)
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $${PWD}/../../includes

DESTDIR = $${OUT_PWD}/../../plugins

include("../../config.pri")

target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    CRL/Car2PCSerial/Car2PCSerial.cpp \
    car2pcplugin.cpp

HEADERS += \
    CRL/Car2PCSerial/Car2PCSerial.h \
    CRL/common.h \
    car2pcplugin.h

DISTFILES += \
    config.json

RESOURCES += \
    qml.qrc
