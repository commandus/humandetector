#!/bin/sh
for i in {1..5000}; do 
	date +%s%N | cut -b1-13
done
