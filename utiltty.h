#ifndef UTIL_TTY_H
#define UTIL_TTY_H     1

/**
 * utiltty
 * 
 * openDevice()- open Vega device serial port
 * 
 * closeDevice()- close Vega device serial port
 * 
 */ 

#include <string>

int openDevice
(
	const std::string &deviceName
);

void closeDevice(
	int fd
);

#endif
