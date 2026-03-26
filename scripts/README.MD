## headunit-desktop install scripts ##

**builddep.sh**

Build dependencies installation, must be run as root

**build.sh**

Build script

**install.sh**

 - Install script to setup hud user, pipewire, wireplumber, wifi and bluetooth.
 - Needs to be run as root.
 - View the script variable to customise to your installation.

**headunit.service**

Systemd user service file to run under the user "hud".

**HeadUnit Desktop.conf**

Example configuration.

**ofono.conf**

DBus permissions for ofono for user "hud"
