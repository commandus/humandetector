#include "util-cmd.h"
#include <cstring>
#include <stdlib.h>

/**
 * @see https://www.gnu.org/software/libc/manual/html_node/Wordexp-Example.html
 */

char **string2argv(
	wordexp_t **we,
	int &argc,
	const std::string &cmdline
)
{
	if (wordexp(cmdline.c_str(), *we, 0))
		return NULL;
	argc = (*we)->we_wordc;
	return (*we)->we_wordv;
}

void argvFree(
	wordexp_t *we
) {
	wordfree(we);
}
