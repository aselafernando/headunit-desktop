#!/bin/bash

apt-get -y install build-essential automake git cmake \
qt6-base-dev qtchooser qmake6 qt6-base-dev-tools qt6-declarative-dev qt6-multimedia-dev \
libqt6bluetooth6 qt6-connectivity-dev qt6-charts-dev qt6-serialport-dev \
qml6-module-qtquick qml6-module-qtquick-layouts qml6-module-qtquick-dialogs qml6-module-qtquick-controls \
qml6-module-qtquick-window qml6-module-qtmultimedia qml6-module-qt-labs-settings \
qml6-module-qt-labs-folderlistmodel qml6-module-qt-labs-platform libqt6bluetooth6 qml6-module-qtcharts \
qml6-module-qt5compat-graphicaleffects \
qt6-connectivity-dev qml6-module-qtquick-controls \
libqt6charts6 qt6-charts-dev qml6-module-qtcharts qt6-serialport-dev \
gstreamer1.0-qt6 \
libkf6bluezqt-dev qt6-5compat-dev \
libkf6pulseaudioqt-dev

## Audio packages
apt-get -y install pipewire wireplumber pipewire-audio gstreamer1.0-pipewire

## Bluetooth codecs
apt-get -y install libfreeaptx0 libsbc1 libaacs0 libspa-0.2-bluetooth libvo-aacenc0 libldacbt-abr2 libldacbt-enc2 liblc3-1 libopus0 fdkaac

## Volume control plugin
apt-get -y install libpulse-dev

## I2C light sensor plugin
apt-get -y install libi2c-dev

## USB connection listener plugin
apt-get -y install libusb-dev

## GPSD plugin
apt-get -y install libgps-dev gpsd

## Welle.IO DAB plugin
apt-get -y install libfftw3-dev libmpg123-dev libfaad-dev librtlsdr-dev libairspy-dev libsoapysdr-dev libmp3lame-dev
