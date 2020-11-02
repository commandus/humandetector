#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <signal.h>

#include "argtable3/argtable3.h"

#include "errlist.h"

const std::string progname = "usb-rfid-card";
#define DEF_CONFIG_FILE_NAME ".usb-rfid-card"

static void done()
{
 // destroy and free all
  exit(0);
}

static void stop()
{

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

class UsbKeyboardOptions {
private:
public:
  bool stopped;
  bool repeatadly;
  UsbKeyboardOptions() 
    : stopped(false), repeatadly(false)
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
  UsbKeyboardOptions &options,
	int argc,
	char* argv[]
)
{
  // sigur db connection options
  struct arg_lit *a_repeatadly = arg_lit0("r", "read", "read lines from stdin");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
    a_repeatadly,
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

  options.repeatadly = a_repeatadly->count > 0;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Sigur RFID card keyboard" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

int run (
  std::istream &strmin,
  std::ostream &strmout,
  UsbKeyboardOptions &options
) {
    int cardno;
    while (!options.stopped && strmin.good()) {
      // std::string line;
      // std::getline(strmin, line);

      std::cin >> cardno;
      if (strmin.eof())
        break;

      time_t t = time(NULL);
      strmout << " --card " << cardno << " --timein " << t << std::endl;
      strmout.flush();
    }
}

UsbKeyboardOptions options;

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

  int c;
  if (options.repeatadly) {
    c = run(std::cin, std::cout, options);
  } else {
    std::cin >> c;
    std::cout << c << std::endl;
  }
  return OK;
}