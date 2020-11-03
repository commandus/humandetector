#include "utilstring.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

static std::string file2string(std::istream &strm)
{
	if (!strm)
		return "";
	return std::string((std::istreambuf_iterator<char>(strm)), std::istreambuf_iterator<char>());
}

std::string file2string(const char *filename)
{
	if (!filename)
		return "";
	std::ifstream t(filename);
	return file2string(t);
}

/**
 * config file consists of lines:
 *  conninfo
 */
int parseConfigDb(
    std::string &conninfo,
    const std::string &config
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s)) {
        conninfo = s;
    } else {
        conninfo = "";
    }
	return 0;
}
