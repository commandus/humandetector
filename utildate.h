#ifndef UTIL_DATE_H_
#define UTIL_DATE_H_	1

#include <string>
#include <inttypes.h>

/**
 * @param ms -1: do not add to the tail of formatted date
 */
std::string ltimeString(
	time_t value,
	int ms,
	const std::string &format
);

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v);

#endif