#ifndef UTIL_DATE_H_
#define UTIL_DATE_H_	1

#include <string>
#include <inttypes.h>

std::string ltimeString(time_t value, const std::string &format);

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v);

#endif