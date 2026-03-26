#!/bin/bash

DESTDIR=/opt/hud
NUMTHREADS=4

git clone --recursive -b qt6 https://github.com/aselafernando/headunit-desktop.git
cd headunit-desktop
protoc --proto_path=./plugins/android-auto/headunit --cpp_out=./plugins/android-auto/headunit/src/protocol AndroidAuto.proto
protoc --proto_path=./plugins/android-auto/headunit --cpp_out=./plugins/android-auto/headunit/src/protocol Bluetooth.proto
mv ./plugins/android-auto/headunit/src/protocol/*.h ./plugins/android-auto/headunit/includes/protocol/
mkdir build
cd build
qmake PREFIX=${DESTDIR} ../headunit-desktop.pro
make -j${NUMTHREADS}
make install
