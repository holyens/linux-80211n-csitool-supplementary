#!/usr/bin/sudo /bin/bash
set -x
modprobe -r iwldvm iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
ifconfig wlan0 2>/dev/null 1>/dev/null
{ set +x; } 2>/dev/null
while [ $? -ne 0 ]
do
    echo -n "."
    ifconfig wlan0 2>/dev/null 1>/dev/null
done
echo ""
set -x
iw dev wlan0 interface add mon0 type monitor
ifconfig mon0 up
iw dev mon0 set channel $1 $2
{ set +x; } 2>/dev/null
if [[ $2 == HT40* ]] ; then
    set -x
    echo 0x4901 | sudo tee `sudo find /sys -name monitor_tx_rate`
    { set +x; } 2>/dev/null
else
    set -x
    echo 0x4101 | sudo tee `sudo find /sys -name monitor_tx_rate`
    { set +x; } 2>/dev/null
fi
echo "done"

