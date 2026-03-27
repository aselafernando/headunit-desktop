#!/bin/bash

DESTDIR=/opt/hud
NUMTHREADS=$(grep -c ^processor /proc/cpuinfo)
KARCH=$(uname -m)

mkdir build
cd build
if [ "$KARCH" = "aarch64" ]; then
    qmake PREFIX=${DESTDIR} CONFIG+=rpi ../../headunit-desktop.pro
else
    qmake PREFIX=${DESTDIR} ../../headunit-desktop.pro
fi
make -j${NUMTHREADS}
make install
