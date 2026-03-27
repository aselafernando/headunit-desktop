#!/bin/bash

DESTDIR=/opt/hud
NUMTHREADS=4

git clone --recursive https://github.com/aselafernando/headunit-desktop.git
cd headunit-desktop
mkdir build
cd build
qmake PREFIX=${DESTDIR} ../headunit-desktop.pro
make -j${NUMTHREADS}
make install
