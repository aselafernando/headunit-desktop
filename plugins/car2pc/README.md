## Description
Plugin to allow the use of car2pc adaptors sold by Grom audio (the adaptor plugs into the cd changer port of stock head units and presents it as a serial interface) with [headunit-desktop](https://github.com/viktorgino/headunit-desktop)

## Instructions
1. Clone repository within the 'modules' folder of headunit-desktop.
2. Modify headunit-desktop/headunit-desktop.pro and add in a line to include modules/Car2PC as described [here](https://github.com/viktorgino/headunit-desktop/wiki/Plugin-System)
3. Compile and install

## UDEV Rules
First because the car2pc adaptors are not associated in the linux kernel as being able to use the ftdi usb serial driver, we setup a rule to force this.
I have not patched the upstream kernel as I believe the PID used by these products (0xABCF) is a test PID and not formally issued by FTDI.
```
ACTION=="add", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="abcf", RUN+="/sbin/modprobe ftdi_sio" RUN+="/bin/sh -c 'echo 0403 abcf > /sys/bus/usb-serial/drivers/ftdi_sio/new_id'"
```
Second, we setup a rule to always place the adaptor at /dev/car2pc, the default location the plugin will look for it.
```
SUBSYSTEM=="tty", SUBSYSTEMS=="usb", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="abcf", SYMLINK+="car2pc"
```
Update udev
```
udevadm control --reload
```
Now plug in your adaptor (or remove and re-attach it). It should now be at /dev/car2pc.
