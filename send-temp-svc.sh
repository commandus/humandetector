#!/bin/sh
exec 6<&0
echo "--a 1" | ./usb-rfid-card -r -k 6
exec 0<&6 6<&-
exit 0
