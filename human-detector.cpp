#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef _MSC_VER
#define DEF_DEVICE_PATH "COM1"
#else
#include <signal.h>
#define DEF_DEVICE_PATH "/dev/ttyACM0"
#endif

#include "argtable3/argtable3.h"

#include "errlist.h"

static bool stopped = false;

const std::string progname = "human-detector";
#define  DEF_CONFIG_FILE_NAME ".human-detector"

static void done()
{
 // destroy and free all
  exit(0);
}

static void stop()
{
  stopped = true;
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

class DetectorOptions {
  public:
    std::string path;
    int temperature0;
    int temperature1;
    int dt;
    int verbosity;
    DetectorOptions() :
      path(""), temperature0(32), temperature1(42), dt(1), verbosity(0)
    {
    }
};

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd
(
  DetectorOptions &detectorOptions,
	int argc,
	char* argv[]
)
{
  // device path
  struct arg_str *a_path = arg_str0(NULL, NULL, "COM port path", "Default " DEF_DEVICE_PATH);
  // temperature
  struct arg_int *a_t0 = arg_int0("0", "t0", "<number>", "Default 32");
  struct arg_int *a_t1 = arg_int0("1", "t1", "<number>", "Default 42");
  // time window
  struct arg_int *a_window = arg_int0("s", "seconds", "<number>", "Default 1");
  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_t0, a_t1, a_window,
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

  if (a_path->count) {
     detectorOptions.path = *a_path->sval;
  }

  if (detectorOptions.path.empty()) {
    detectorOptions.path = DEF_DEVICE_PATH;
  }

  detectorOptions.verbosity = a_verbosity->count;

  if (a_t0->count) {
     detectorOptions.temperature0 = *a_t0->ival;
  }

  if (a_t1->count) {
     detectorOptions.temperature1 = *a_t1->ival;
  }

  if (detectorOptions.temperature1 < detectorOptions.temperature0) {
    std::cerr << "Temperature 1 smaller temperature 0." << std::endl;
    nerrors++;
  }

  if (a_window->count) {
    detectorOptions.dt = *a_window->ival;
  }

  if (detectorOptions.dt <= 0) {
    std::cerr << "Time window is too small." << std::endl;
    nerrors++;
  }

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Command line vega client" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int main(
  int argc,
	char* argv[]
) {
  
  DetectorOptions options;
  if (parseCmd(options, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };
  int r = 0;
  return r;
}
