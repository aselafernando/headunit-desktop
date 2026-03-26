#!/bin/bash

HUD_USER=hud
HUD_GRP=${HUD_USER}
HUD_HOME=/var/lib/${HUD_USER}
HUD_INSTALL_DIR=/opt/hud
NAME=HUD
WLAN_INTERFACE=wlan0
WLAN_KEY=$(cat /dev/urandom | tr -dc 'A-Za-z0-9@#' | head -c 16)
WLAN_COUNTRY=AU

karch=$(uname -m)

mkdir -p ${HUD_INSTALL_DIR}

#Bluetooth
sed -i "/#Name/s/#//;s/BlueZ/${NAME}/" /etc/bluetooth/main.conf

#Copy service files to systemd
cp headunit.service /usr/lib/systemd/user/

cp ofono.conf /etc/dbus-1/system.d/ofono.conf

#Pipewire Realtime
echo "@pipewire soft nice -11" > /etc/security/limits.d/99-pipewire.conf
echo "@pipewire hard nice -11" >> /etc/security/limits.d/99-pipewire.conf
echo "@pipewire soft rtprio 95" >> /etc/security/limits.d/99-pipewire.conf
echo "@pipewire hard rtprio 95" >> /etc/security/limits.d/99-pipewire.conf

#Create user for HUD
adduser --system --no-create-home --group ${HUD_USER}
HUD_UID=$(id -u ${HUD_USER})

usermod -aG input ${HUD_USER}
usermod -aG video ${HUD_USER}
usermod -aG render ${HUD_USER}
usermod -aG dialout ${HUD_USER}
usermod -aG plugdev ${HUD_USER}
usermod -aG bluetooth ${HUD_USER}
usermod -aG audio ${HUD_USER}
usermod -aG i2c ${HUD_USER}
usermod -aG pipewire ${HUD_USER}
usermod -d ${HUD_HOME} ${HUD_USER}
mkdir -p ${HUD_HOME}/.config

#Ensure DBUS paths are set correctly for services
sed -i "s/Slice=session.slice/Slice=session.slice\nEnvironment=XDG_RUNTIME_DIR=\/run\/user\/%U\nEnvironment=DBUS_SESSION_BUS_ADDRESS=unix:path=\/run\/user\/%U\/bus/" \
	/usr/lib/systemd/user/pipewire.service
sed -i "s/Slice=session.slice/Slice=session.slice\nEnvironment=XDG_RUNTIME_DIR=\/run\/user\/%U\nEnvironment=DBUS_SESSION_BUS_ADDRESS=unix:path=\/run\/user\/%U\/bus/" \
	/usr/lib/systemd/user/wireplumber.service

#https://wiki.archlinux.org/title/WirePlumber#Keep_Bluetooth_running_after_logout_/_Headless_Bluetooth
mkdir -p ${HUD_HOME}/.config/wireplumber/wireplumber.conf.d

cat > ${HUD_HOME}/.config/wireplumber/wireplumber.conf.d/disable-logind.conf<< EOF
wireplumber.profiles = {
  main = {
    monitor.bluez.seat-monitoring = disabled
  }
}
EOF

cat > ${HUD_HOME}/.config/wireplumber/wireplumber.conf.d/bluetooth.conf<< EOF
monitor.bluez.properties = {
  bluez5.roles = [ a2dp_sink bap_sink hfp_hf hsp_hs ]
  bluez5.codecs = [ sbc sbc_xq aac ldac aptx aptx_hd aptx_ll aptx_ll_duplex faststream faststream_duplex ]
  bluez5.enable-sbc-xq = true
  bluez5.enable-msbc = true
  bluez5.hfphsp-backend = "ofono"
  bluez5.enable-hw-volume = false
  bluez5.default.rate = 48000
  bluez5.a2dp.ldac.quality = "hq"
}
EOF

mkdir -p ${HUD_HOME}/.config/pipewire/pipewire.conf.d/

cat > ${HUD_HOME}/.config/pipewire/pipewire.conf.d/rt-priority.conf<< EOF
{
    "module.rt.args": {
        "rt.prio": 95
    }
}
EOF

cat > ${HUD_HOME}/.config/pipewire/pipewire.conf.d/bluetooth.conf<< EOF
context.properties = {
    default.clock.allowed-rates = [ 192000 96000 48000 44100 ]
    default.clock.quantum = 256
    default.clock.min-quantum = 128
    default.clock.max-quantum = 1024
}
EOF

cat > ${HUD_HOME}/.bashrc<< EOF
export XDG_RUNTIME_DIR=/run/user/${HUD_UID}
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${HUD_UID}
EOF

chown -R ${HUD_USER}:${HUD_GRP} ${HUD_HOME}

systemctl daemon-reload

#Using linger means we dont have to manually setup a dbus session, can use pipewire and wireplumber as is
loginctl enable-linger ${HUD_USER}

#sudo -u hud XDG_RUNTIME_DIR=/run/user/${HUD_UID} DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${HUD_UID} systemctl --user daemon-reload
#sudo -u hud XDG_RUNTIME_DIR=/run/user/${HUD_UID} DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${HUD_UID} systemctl --user enable obex
#sudo -u hud XDG_RUNTIME_DIR=/run/user/${HUD_UID} DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/${HUD_UID} systemctl --user enable headunit

echo "i2c-dev" > /etc/modules-load.d/i2c.conf

if [ "$karch" = "aarch64" ]; then
    #WiFi
    nmcli c add type wifi ifname ${WLAN_INTERACE} con-name ${NAME} autoconnect yes ssid ${NAME}
    nmcli c mod ${NAME} 802-11-wireless.mode ap 802-11-wireless.band a 802-11-wireless.hidden false 802-11-wireless.powersave 2
    #Less chance of interferrence on higher channels
    nmcli c mod ${NAME} 802-11-wireless.channel 149
    nmcli c mod ${NAME} ipv4.method manual ipv4.address 192.168.30.1/24
    nmcli c mod ${NAME} wifi-sec.key-mgmt wpa-psk
    nmcli c mod ${NAME} 802-11-wireless-security.proto rsn 802-11-wireless-security.pairwise ccmp 802-11-wireless-security.group ccmp
    nmcli c mod ${NAME} wifi-sec.psk ${WLAN_KEY}
else

cat > /etc/wpa_supplicant/wpa_supplicant-${WLAN_INTERFACE}.conf<< EOF
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=${WLAN_COUNTRY}
fast_reauth=1
ap_scan=2
network={
  ssid="${NAME}"
  mode=2
  key_mgmt=WPA-PSK
  proto=RSN
  psk="${WLAN_KEY}"
  frequency=5745
  ht40=1
}
EOF

echo "allow-hotplug ${WLAN_INTERFACE}" >> /etc/network/interfaces
echo "iface ${WLAN_INTERFACE} inet static"  >> /etc/network/interfaces
echo "address 192.168.30.1" >> /etc/network/interfaces
echo "netmask 255.255.255.0" >> /etc/network/interfaces

systemctl enable wpa_supplicant@${WLAN_INTERFACE}

fi

WLAN_MAC=$(ip -o link show ${WLAN_INTERFACE} | awk '{print $17}' | cut -d/ -f1)

sed -i "s/adapterName=.*/adapterName=${NAME}/" ${HUD_HOME}/.config/HeadUnit Desktop/HeadUnit\ Desktop.conf
sed -i "s/wlan_bssid=.*/wlan_bssid=${WLAN_MAC}/" ${HUD_HOME}/.config/HeadUnit Desktop/HeadUnit\ Desktop.conf
sed -i "s/wlan_password=.*/wlan_password=${WLAN_KEY}/" ${HUD_HOME}/.config/HeadUnit Desktop/HeadUnit\ Desktop.conf
sed -i "s/wlan_name=.*/wlan_name=${NAME}/" ${HUD_HOME}/.config/HeadUnit Desktop/HeadUnit\ Desktop.conf
sed -i "s/head_unit_name=.*/head_unit_name=${NAME}/" ${HUD_HOME}/.config/HeadUnit Desktop/HeadUnit\ Desktop.conf

sed -i "s/#dhcp-range=.*/dhcp-range=192.168.30.50,192.168.30.150,12h/" /etc/dnsmasq.conf
sed -i "s/#interface=.*/interface=${WLAN_INTERFACE}/" /etc/dnsmasq.conf
sed -i "s/#dhcp-option=option:router,.*/dhcp-option=option:router,0.0.0.0/" /etc/dnsmasq.conf


if [ "$karch" = "aarch64" ]; then
    rfkill unblock all
fi

systemctl enable bluetooth
systemctl enable dnsmasq
systemctl enable ofono

