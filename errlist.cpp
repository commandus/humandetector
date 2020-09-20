#include <string.h>
#include "errlist.h"

#define ERR_COUNT 4

static const char* errlist[ERR_COUNT] = {
  ERR_COMMAND_LINE,
  ERR_OPEN_DEVICE,
  ERR_CLOSE_DEVICE,
  ERR_BAD_STATUS
};

const char *strerror_humandetector(
  int errcode
)
{
  if ((errcode <= -500) && (errcode >= -500 - ERR_COUNT)) {
    return errlist[-(errcode + 500)];
  }
  return strerror(errcode);
}
