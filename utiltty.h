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

/**
 * 
 * Open Vega device serial port
 * 
 * @param deviceName e.g. "/dev/ttyACM0"
 * @param timeout seconds
 * 
 */ 
int openDevice
(
	const std::string &deviceName,
	int timeout
);

void closeDevice(
	int fd
);

#endif
