#include "utiltty.h"

#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int openDevice
(
	const std::string &deviceName,
	int timeout
)
{
	int flags = O_RDWR;
	int paritycheck = 0;
	int flagspeed = B115200;

	// Open serial port and set default options
	flags |= O_NOCTTY;
	/*
	if (timeout > 0)
		flags |= O_NDELAY;	// Error read 11: Resource temporarily unavailable
	*/
	int mDevice = open(deviceName.c_str(), flags);
	
	if (mDevice == -1) {
		return -1;
	}

	struct termios newtio;
	memset(&newtio, 0, sizeof(newtio));

	int r = tcgetattr(mDevice, &newtio);

	newtio.c_cflag = CS8 | CLOCAL | CREAD; // | PARENB | PARODD;	// 20150921
	if (paritycheck == 0)
		newtio.c_cflag = newtio.c_cflag | PARENB | PARODD;
	else
		if (paritycheck == 1)
			newtio.c_cflag |= PARENB;
	
	newtio.c_iflag &= ~ICRNL;
	// // newtio.c_iflag = IGNPAR | IGNBRK;  // ignore bytes with parity errors otherwise make device raw (no other input processing)
	// newtio.c_iflag = 0
	// Raw output
	newtio.c_oflag = 0;
	// initialize all control characters default values can be found in /usr/include/termios.h, and are given in the comments, but we don't need them here
	if (timeout == 0)
	{
		newtio.c_cc[VTIME] = 0;		// read returns immediately 2or
		newtio.c_cc[VMIN] = 1;		// until 1 character arrives
	}
	else
	{
		newtio.c_cc[VTIME] = timeout;	// read returns after timeout * 0.1s or
		newtio.c_cc[VMIN] = 1;		// until 1 character arrives
	}
    // ICANON  : enable canonical input disable all echo functionality, and don't send signals to calling program
    newtio.c_lflag = 0;

	cfsetispeed(&newtio, flagspeed);
	cfmakeraw(&newtio);

	tcflush(mDevice, TCIFLUSH);                     // clean line
	r = tcsetattr(mDevice, TCSANOW, &newtio);       // activate the settings for the port
	//
	if (timeout > 0)
		fcntl(mDevice, F_SETFL, fcntl(mDevice, F_GETFL) & ~O_NONBLOCK);
	return mDevice;	
}

void closeDevice(
	int fd
)
{
	close(fd);
}
