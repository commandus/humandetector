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
    uint64_t &gateid,
    uint64_t &secret,
    const std::string &config
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s)) {
        conninfo = s;
    }
    if (std::getline(ss, s)) {
        gateid = strtoull(s.c_str(), NULL, 10);
    }
	if (std::getline(ss, s)) {
        secret = strtoull(s.c_str(), NULL, 10);
    }
	return 0;
}

/**
 * config file consists of lines:
 *  URL
 *  gate id number
 *  gate secret number
 */
int parseConfigJson(
    std::string &url,
    uint64_t &gateid,
    uint64_t &secret,
    const std::string &config
) {
    std::stringstream ss(config);
    std::string s;
    if (std::getline(ss, s)) {
        url = s;
    }
    if (std::getline(ss, s)) {
        gateid = strtoull(s.c_str(), NULL, 10);
    }
	if (std::getline(ss, s)) {
        secret = strtoull(s.c_str(), NULL, 10);
    }
	return 0;
}

