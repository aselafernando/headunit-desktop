## Description
Plugin to allow the use of J2534 cables with [headunit-desktop](https://github.com/viktorgino/headunit-desktop)

## Instructions
1. Clone repository within the 'modules' folder of headunit-desktop.
2. Modify headunit-desktop/headunit-desktop.pro and add in a line to include modules/J2534 as described [here](https://github.com/viktorgino/headunit-desktop/wiki/Plugin-System)
3. Compile and install

## J2534
This plugin is written to suit Toyota 1ZZ engines using a MiniVCI adapter
It requires libj2534.so be placed in the run directory of headunit-desktop where it can be dynamically loaded at runtime
