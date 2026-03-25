# HeadUnit Desktop

THis is a fork of the original [headunit-desktop](https://github.com/viktorgino/headunit-desktop)

HeadUnit Desktop is a Qt6 based free and open source software that is intended to be run on computers built into cars. HUD is designed to be highly modular and easy to extend even for beginners.

This software is currently under active development and lot of the features are experimental. The main features are currently:

 - Media player with a media library and media scanner
 - Android Auto™ client (wired & wireless)
 - DAB radio
 - Car2PC device support
 - GPSD support
 - J2534 support
 - Reverse camera support

Reverse camera and android auto leverage a gstreamer pipeline with OpenGL rendering to maximise hardware decoding. This is especially required for smooth performance on low power systems like a Rapsberry Pi.

It is currently optimised for a 1024x600 screen.

# Compiling on Debian 13

## Build tools
`apt-get -y install build-essential automake git cmake`

## Qt6 packages

`qt6-base-dev qtchooser qmake6 qt6-base-dev-tools qt6-declarative-dev qt6-multimedia-dev
libqt6bluetooth6 qt6-connectivity-dev qt6-charts-dev qt6-serialport-dev
qml6-module-qtquick qml6-module-qtquick-layouts qml6-module-qtquick-dialogs qml6-module-qtquick-controls
qml6-module-qtquick-window qml6-module-qtmultimedia qml6-module-qt-labs-settings
qml6-module-qt-labs-folderlistmodel qml6-module-qt-labs-platform libqt6bluetooth6 qml6-module-qtcharts
qml6-module-qt5compat-graphicaleffects
qt6-connectivity-dev qml6-module-qtquick-controls
libqt6charts6 qt6-charts-dev qml6-module-qtcharts qt6-serialport-dev
gstreamer1.0-qt6
libkf6bluezqt-dev qt6-5compat-dev
libkf6pulseaudioqt-dev`

## Audio packages
`apt-get -y install pipewire wireplumber pipewire-audio libspa-0.2-bluetooth gstreamer1.0-pipewire`

## Bluetooth codecs
`apt-get -y install libfreeaptx0 libsbc1 libaacs0 libvo-aacenc0 libldacbt-abr2 libldacbt-enc2 liblc3-1 libopus0 fdkaac`

## Volume control plugin
`apt-get -y install libpulse-dev`

## I2C light sensor plugin
`apt-get -y install libi2c-dev`

## USB connection listener plugin
`apt-get -y install libusb-dev`

## GPSD plugin
`apt-get -y install libgps-dev gpsd`

## Welle.IO DAB plugin
`apt-get -y install libfftw3-dev libmpg123-dev libfaad-dev librtlsdr-dev libairspy-dev libsoapysdr-dev libmp3lame-dev`

## Compile Instructions

1. Install the dependencies above
2. Run the following
```
git clone --recursive -b qt6 https://github.com/aselafernando/headunit-desktop.git
cd headunit-desktop
protoc --proto_path=./plugins/android-auto/headunit --cpp_out=./plugins/android-auto/headunit/src/protocol AndroidAuto.proto
protoc --proto_path=./plugins/android-auto/headunit --cpp_out=./plugins/android-auto/headunit/src/protocol Bluetooth.proto
mv ./plugins/android-auto/headunit/src/protocol/*.h ./plugins/android-auto/headunit/includes/protocol/
mkdir build
cd build
qmake PREFIX=***DESTINATION DIR*** ../headunit-desktop.pro
make
make install
```
