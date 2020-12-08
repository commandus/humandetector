#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <sys/time.h>
#include <signal.h>

#include "utiltty.h"
#include "utildate.h"
#include "wt-sigur.h"

#include "argtable3/argtable3.h"

#include "errlist.h"

#define DEF_DB_HOST       "sccm-dp.ad.ysn.ru"
#define DEF_DB_USER       "root"
#define DEF_DB_PASSWD     ""
#define DEF_AREA  0
#define DEF_DIRECTION  2

const std::string progname = "sigur-last-card";
#define DEF_CONFIG_FILE_NAME ".sigur-last-card"
#define DEF_TIME_FORMAT      "%F %T"

static SigurOptions options;

static void done()
{
 // destroy and free all
  exit(0);
}

static void stop()
{
  options.stopped = true;
}

void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
    done();
		break;
	default:
		break;
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
}
#endif

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd
(
  SigurOptions &options,
	int argc,
	char* argv[]
)
{
  // sigur db connection options
  struct arg_str *a_dbhost = arg_str0("h", "host", "<address>", "Sigur database host name or address " DEF_DB_HOST);
  struct arg_str *a_dbuser = arg_str0("u", "user", "<name>", "Sigur database user name. Default " DEF_DB_USER);
  struct arg_str *a_dbpasswd = arg_str0("w", "passwd", "<secret>", "Sigur database user password. Default " DEF_DB_PASSWD);
  struct arg_int *a_dbport = arg_int0("p", "port", "<number>", "Database TCP port number. Default 3305");

  struct arg_int *a_area = arg_int0("a", "area", "<number>", "-1: any. Default 0");
  struct arg_int *a_direction = arg_int0("d", "direction", "<number>", "1- out, 2- in, 0- any. Default 2(in)");

  struct arg_lit *a_repeatadly = arg_lit0("R", "no-read", "do not read lines from stdin");
  struct arg_str *a_timeFormat = arg_str0("t", "timeformat", "<format>", "output time format, e.g. \"" DEF_TIME_FORMAT "\". Default prints seconds since Unix epoch ");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_dbhost, a_dbuser, a_dbpasswd, a_dbport,
    a_area, a_direction, a_repeatadly, a_timeFormat,
    a_verbosity, a_help, a_end 
	};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

  if (a_dbhost->count) {
     options.host = *a_dbhost->sval;
  }
  if (options.host.empty()) {
    options.host = DEF_DB_HOST;
  }
  if (a_dbuser->count) {
     options.user = *a_dbuser->sval;
  }
  if (options.user.empty()) {
    options.user = DEF_DB_USER;
  }
  if (a_dbpasswd->count) {
     options.passwd = *a_dbpasswd->sval;
  } else {
    options.passwd = DEF_DB_PASSWD;
  }
  if (a_dbport->count) {
     options.port = *a_dbport->ival;
  } else {
    options.port = 3305;
  }
  if (a_area->count) {
     options.area = *a_area->ival;
  } else {
    options.area = DEF_AREA;
  }
  if (a_direction->count) {
     options.direction = *a_direction->ival;
  } else {
    options.direction = DEF_DIRECTION;
  }

  options.repeatadly = a_repeatadly->count == 0;

  if (a_timeFormat->count) {
     options.timeFormat = *a_timeFormat->sval;
  } else {
    options.timeFormat = DEF_TIME_FORMAT;
  }

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Sigur last card" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static std::string formatTime(
  const SigurOptions &options,
  time_t value
) {
  if (options.timeFormat.empty()) {
    std::stringstream ss;
    ss << std::dec << value;
    return ss.str();
  }
  return ltimeString(value, -1, options.timeFormat);
}

int run (
  std::istream &strmin,
  std::ostream &strmout,
  SigurOptions &options
) {
    while (!options.stopped && strmin.good()) {
      std::string line;
      std::getline(strmin, line);
      if (strmin.eof())
        break;
      time_t t;
      int cardno = options.getLastSigurEmployee(t);
      if (cardno < 0) {
        if (cardno != ERR_CODE_DB_EMPTY) {
          std::cerr << "Error " << cardno << ": " << strerror_humandetector(cardno) << std::endl;
          // try to connect to the database
          options.disconnect();
          options.reconnect();
          // try again
          cardno = options.getLastSigurEmployee(t);
        }
      }
      strmout << line << " --card " << cardno << " --timein " << t << std::endl;
      strmout.flush();
    }
}

int main(
  int argc,
	char* argv[]
) {
  
  if (parseCmd(options, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };

#ifdef _MSC_VER
#else  
  setSignalHandler();
#endif

  options.reconnect();
  int c;
  if (options.repeatadly) {
    c = run(std::cin, std::cout, options);
    if (c < 0) {
      exit(c);
    }
  } else {
    time_t t;
    c = options.getLastSigurEmployee(t);
    if (c < 0) {
      if (c != ERR_CODE_DB_EMPTY) {
        std::cerr << "Error " << c << ": " << strerror_humandetector(c) << std::endl;
      }
      exit(c);
    }
    std::cout << c << "\t" << formatTime(options, t) << std::endl;
  }
  return OK;
}

