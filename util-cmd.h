/**
 * @see https://stackoverflow.com/questions/1706551/parse-string-into-argv-argc
 */

#include <string>
#include <wordexp.h>

char **string2argv(
	wordexp_t **we,
	int &argc,
	const std::string &cmdline
);

void argvFree(
	wordexp_t *we
);
