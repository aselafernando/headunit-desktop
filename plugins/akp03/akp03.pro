TEMPLATE = lib
CONFIG += c++17 plugin link_pkgconfig
QT += quick
TARGET = $$qtLibraryTarget(akp03e-plugin)
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $${PWD}/../../includes

DESTDIR = $${OUT_PWD}/../../plugins

include("../../config.pri")

target.path = $${PREFIX}/plugins

INSTALLS += target

SOURCES += \
    akp03device.cpp \
    akp03plugin.cpp

HEADERS += \
    third_party/stb_image.h \
    third_party/stb_image_write.h \
    akp03device.h \
    akp03plugin.h

DISTFILES += \
    config.json

RESOURCES += \
    akp03.qrc
