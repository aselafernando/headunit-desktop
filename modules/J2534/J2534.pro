TEMPLATE = lib
CONFIG += c++11 plugin link_pkgconfig
QT += quick
TARGET = $$qtLibraryTarget(j2534-plugin)
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $${PWD}/../../includes
DESTDIR = $${OUT_PWD}/../../plugins

QML_IMPORT_PATH += $$OUT_PWD

RESOURCES += \
    qml.qrc

include("../../config.pri")

target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    j2534plugin.cpp

HEADERS += \
    j2534plugin.h

DISTFILES += \
    config.json

