#ifndef UTILSTRING_H_
#define UTILSTRING_H_	1

#include <string>
#include <inttypes.h>

// read file
std::string file2string(const char *filename);

void string2file(const std::string &filename, const std::string &value);

/**
 * config file consists of lines:
 *  conninfo
 *  gate id number
 *  gate secret number
 */
int parseConfigDb(
    std::string &conninfo,
    uint64_t &gateid,
    uint64_t &secret,
    const std::string &config
);

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
);

#endif
