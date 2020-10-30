#include "utildate.h"
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "platform.h"

#ifndef _MSC_VER
#include "strptime.h"
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#endif

const static char *dateformat = "%FT%T";

std::string ltimeString(
	time_t value,
	int ms,
	const std::string &format
) {
	if (!value)
		value = time(NULL);
	struct tm tm;
	localtime_s(&tm, &value);
	char dt[64];
	strftime(dt, sizeof(dt), format.c_str(), &tm);
	if (ms == -1)
		return std::string(dt);
	else {
		std::stringstream ss;
		ss << dt << "." << std::setw(3) << std::setfill('0') << ms;
		return ss.str();
	}
}

/**
 * Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t parseDate(const char *v)
{
	struct tm tmd;
	memset(&tmd, 0, sizeof(struct tm));

	time_t r;
	if (strptime(v, dateformat, &tmd) == NULL)
			r = strtol(v, NULL, 0);
	else
			r = mktime(&tmd);
	return r;
}
