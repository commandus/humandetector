#!/bin/bash
for i in {1..5000}; do 
	d=`date +%s%N | cut -b1-13`
	echo $1 $d $2 $3 $4 $5 $6 $7 $8 $9
	sleep 1
done
