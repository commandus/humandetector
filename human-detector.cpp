#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "utiltty.h"

#ifdef _MSC_VER
#define DEF_DEVICE_PATH "COM1"
#else
#include <signal.h>
#include <unistd.h>
#define DEF_DEVICE_PATH "/dev/ttyACM0"
#endif

#include "argtable3/argtable3.h"

#include "errlist.h"

const std::string progname = "human-detector";
#define  DEF_CONFIG_FILE_NAME ".human-detector"

enum DEGREES {
  DEG_K = 0,
  DEG_C = 1
};

class DetectorOptions {
  public:
    int fd;
    bool stopped;
    std::string path;
    int temperature0;
    int temperature1;
    int dt;
    DEGREES degrees;
    int delay;  // ms
    int verbosity;
    time_t currentTime;
    time_t startTime;
    time_t sentTime;
    int maxT;
    DetectorOptions() :
      fd(0), stopped(false), path(""), temperature0(32), temperature1(42), dt(1),
      degrees(DEG_K), delay(0), verbosity(0), currentTime(0), startTime(0), 
      sentTime(0), maxT(0)
    {
    }
};

static DetectorOptions options;

static void done()
{
 // destroy and free all
  closeDevice(options.fd);
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
  DetectorOptions &detectorOptions,
	int argc,
	char* argv[]
)
{
  // device path
  struct arg_str *a_path = arg_str0(NULL, NULL, "COM port path", "Default " DEF_DEVICE_PATH);
  // temperature
  struct arg_int *a_t0 = arg_int0("l", "t0", "<number>", "lo temperature threshold, C. Default 32");
  struct arg_int *a_t1 = arg_int0("h", "t1", "<number>", "hi temperature threshold, C. Default 42");
  // time window
  struct arg_int *a_window = arg_int0("s", "seconds", "<number>", "Default 1");
  // delay
  struct arg_int *a_delay = arg_int0("y", "delay", "<ms>", "delay to read, C. Default 0");
  struct arg_str *a_degrees = arg_str0("g", "degrees", "K or C", "K- Kelvin (default), C- Celcius");
  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_path, a_t0, a_t1, a_window,
    a_degrees, a_delay, a_verbosity, a_help, a_end 
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

  if (a_degrees->count) {
     std::string d = *a_degrees->sval;
     if (d.length() > 0) {
       switch (d[0])
       {
       case 'C':
       case 'c':
          detectorOptions.degrees = DEG_C;
          break;
       default:
          detectorOptions.degrees = DEG_K;
          break;
       }
     }
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
    std::cerr << "Temperature max threshold value smaller than min." << std::endl;
    nerrors++;
  }

  if (a_window->count) {
    detectorOptions.dt = *a_window->ival;
  }

  if (detectorOptions.dt <= 0) {
    std::cerr << "Time window must be great or equil 0 seconds." << std::endl;
    nerrors++;
  }

  if (a_delay->count) {
      detectorOptions.delay = *a_delay->ival;
  }

  if (detectorOptions.delay < 0) {
    std::cerr << "Delay must be great or equil 0." << std::endl;
    nerrors++;
  }

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Temperature reader" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static size_t sendRequestObjectTemperature(
  const DetectorOptions &options
)
{
  // '0'- object, '1'- ambient, 2- ticks
  if (options.delay > 0) {
#ifdef _MSC_VER
  Sleep(options.delay);
#else
    usleep(options.delay * 1000);
#endif    
  }
  char c = '0';
  return write(options.fd, &c, 1);
}

static size_t parseObjectTemp(
  int &retval,
  std::stringstream &buf
) 
{
  std::string s = buf.str();
  size_t l = s.length();
  if (l > 2) {
    if (s[l - 2] == '\r' && s[l - 1] == '\n') {
      s.erase(l - 2);
      retval = atoi(s.c_str());
      return l;
    }
  }
  return 0;
}

static void putTemperature(
  DetectorOptions &options,
  int temperatureK100
) {
  switch (options.degrees) {
    case DEG_C:
        std::cout << options.currentTime << "\t" 
          << std::fixed << std::setprecision(2)
          << (temperatureK100 - 27315) / 100.0  << std::endl;
      break;
    default:
        std::cout << options.currentTime << "\t" 
          << std::fixed << std::setprecision(2)
          << temperatureK100 / 100.0  << std::endl;
  }
}

static void processWindow(
  DetectorOptions &options,
  int temperature
)
{
  if (options.dt <= 0) {
    putTemperature(options, temperature);
    return;
  }
  // temperature is too low or too high. It is not a human.
  if (temperature < options.maxT) {
    if (options.startTime > 0) {
      // Object is lost
      // Report about last object
      putTemperature(options, temperature);
    } else {
      // Did not see any object yet
      return;
    }
    // reset time
    options.startTime = 0;
    options.sentTime = 0;
    // reset max
    options.maxT = options.temperature0 - 1;
    return;
  }
  // temperature is OK
  if (options.startTime == 0) {
    // New measurement
    options.startTime = time(NULL);
    options.maxT = temperature;
    // Ready for new measurements
    return;
  }

  // Have old measurements

  // Store new max temperature
  if (options.maxT < temperature)
    options.maxT = temperature;
  // Check is it time to report
  if (options.sentTime == 0) {
    if (options.currentTime - options.startTime > options.dt) {
      // need to report
      putTemperature(options, temperature);
      // remember time of last report
      options.sentTime = options.startTime;
    }
  } else {
    // Sent one or more report(s) already.
    if (options.currentTime - options.sentTime > options.dt) {
      // need to re-report
      putTemperature(options, temperature);
      // remember time of last report
      options.sentTime = options.startTime;
    }
  }
}

static void readDevice(
  DetectorOptions &options
)
{
	std::stringstream buf;
	int c = 0;
  options.startTime = time(NULL);
  options.maxT = options.temperature0 - 1;
  sendRequestObjectTemperature(options);
	while (!options.stopped) {
		int r = read(options.fd, &c, 1);
		if (options.verbosity > 2) {
			std::cerr << (char) c;
    }

    buf.put(c);

    int t;
    size_t sz = parseObjectTemp(t, buf);
    if (sz > 0) {
      std::string remains(buf.str().c_str() + sz, buf.str().size() - sz);
      buf.clear(); // Clear state flags.
      buf.str(remains);

      options.currentTime = time(NULL);
      processWindow(options, t);
      sendRequestObjectTemperature(options);
    }
  }
}

int main(
  int argc,
	char* argv[]
) {
  
  if (parseCmd(options, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };
  if (options.verbosity > 1)
    std::cerr << "Device " << options.path << std::endl;

  options.fd = openDevice(options.path);
  if (options.fd < 0) {
    std::cerr << "Error " << ERR_CODE_OPEN_DEVICE << ": " << strerror_humandetector(ERR_CODE_OPEN_DEVICE) << std::endl;
    exit(ERR_CODE_OPEN_DEVICE);
  }

#ifdef _MSC_VER
#else  
  setSignalHandler();
#endif

  // C to 100K
  options.temperature0 = (options.temperature0 * 100) + 27315;
  options.temperature1 = (options.temperature1 * 100) + 27315;

  readDevice(options);

  closeDevice(options.fd);
  if (options.verbosity > 1)
    std::cerr << "Stopped" << std::endl;
  return 0;
}
