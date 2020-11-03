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
 */
int parseConfigDb(
    std::string &conninfo,
    const std::string &config
);

#endif
