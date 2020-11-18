#!/bin/sh
exec 6<&0
#./human-detector --logfile temperature.log /dev/ttyACM1 | ./usb-rfid-card -k 6 | ./put-temperature-db | ./put-temperature-json | jq .
./human-detector --logfile temperature.log /dev/ttyACM1 | ./usb-rfid-card -k 6 | ./put-temperature-db
exec 0<&6 6<&-
exit 0
