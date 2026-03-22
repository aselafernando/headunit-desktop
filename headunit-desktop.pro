TEMPLATE = subdirs

QCXXFLAGS += -DRPI
QCFLAGS += -DRPI

SUBDIRS = \
          app \
#          modules/volume-control \
          modules/phone-bluetooth \
          modules/android-auto \
#          modules/media-player \
          modules/Car2PC \
          modules/J2534 \
          modules/gpsd \
          modules/i2c-light-sensor \
          modules/reversing-camera \
          modules/usbconnectionlistener \
#          modules/welle-io \
#          modules/navit \
#          modules/fm-radio \
#          modules/hvac \
#          modules/sample \
#          modules/rpi \
#          modules/hud-serial \
          themes/default-theme

