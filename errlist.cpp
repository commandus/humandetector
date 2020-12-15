#include <string.h>
#include "errlist.h"

#define ERR_COUNT 16

static const char* errlist[ERR_COUNT] = {
  ERR_COMMAND_LINE,
  ERR_OPEN_DEVICE,
  ERR_CLOSE_DEVICE,
  ERR_BAD_STATUS,
  ERR_DB_NOTCONNECTED,
  ERR_DB_CREATE_DESCRIPTOR,
  ERR_DB_CONNECT,
  ERR_JSON_CONNECT,
  ERR_DB_EXEC_QUERY,
  ERR_DB_GET_RESULT,
  ERR_DB_FETCH_RESULT,
  ERR_DB_EMPTY,
  ERR_WRONG_FILE,
  ERR_INVALID_PAR_DELAY,
  ERR_INVALID_PAR_LOG_FILE,
  ERR_CALIBRATION_DIFF_TOO_BIG
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
