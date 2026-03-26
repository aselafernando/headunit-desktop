TEMPLATE = lib
CONFIG += c++17 plugin link_pkgconfig
QT += quick
TARGET = $$qtLibraryTarget(sample)
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $${PWD}/../../includes
DESTDIR = $${OUT_PWD}/../../plugins

include("../../config.pri")

target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    sampleplugin.cpp

HEADERS += \
    sampleplugin.h

DISTFILES += \
    config.json

RESOURCES += \
    sample.qrc
