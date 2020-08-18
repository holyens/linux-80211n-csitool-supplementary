#!/usr/bin/sudo /bin/bash
set -x
modprobe -r iwldvm iwlwifi mac80211 cfg80211
modprobe iwlwifi connector_log=0x1
# Setup monitor mode, loop until it works
iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
{ set +x; } 2>/dev/null
while [ $? -ne 0 ]
do
    echo -n "."
    iwconfig wlan0 mode monitor 2>/dev/null 1>/dev/null
done
echo ""
set -x
ifconfig wlan0 up
iw dev wlan0 set channel $1 $2
{ set +x; } 2>/dev/null
echo "done"

