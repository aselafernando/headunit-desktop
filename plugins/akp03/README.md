## Description
Plugin to allow the use of AKP03 stream deck clones with [headunit-desktop](https://github.com/aselafernando/headunit-desktop)

## Instructions
1. Clone repository within the 'modules' folder of headunit-desktop.
2. Modify headunit-desktop/headunit-desktop.pro and add in a line to include modules/akp03 as described [here](https://github.com/viktorgino/headunit-desktop/wiki/Plugin-System)
3. Compile and install

## UDEV Rules
Copy the .rules file to /etc/udev/rules.d/ to ensure the device can be read and written by all users.
