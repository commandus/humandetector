#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "utiltty.h"
#include "utildate.h"

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
#define DEF_CONFIG_FILE_NAME ".human-detector"
#define DEF_TIME_FORMAT      "%F %T"

enum DEGREES {
  DEG_K = 0,
  DEG_C = 1
};

enum MODE {
  MODE_IR = 0,
  MODE_AMBIENT = 1,
  MODE_TICK = 2,
  MODE_FILE = 3
};

class DetectorOptions {
  public:
    int fd;
    bool stopped;
    std::string path;
    int temperature0;
    int temperature1;
    int dt;
    enum DEGREES degrees;
    int delay;  // ms
    int verbosity;
    time_t currentTime;
    time_t startTime;
    time_t sentTime;
    int sentTemperature;
    bool printMaxOnly;
    bool reconnect;
    enum MODE mode;
    std::string timeFormat;
    int maxT;
    DetectorOptions() :
      fd(0), stopped(false), path(""), temperature0(32), temperature1(42), dt(1),
      degrees(DEG_C), delay(0), verbosity(0), currentTime(0), startTime(0), 
      sentTime(0), sentTemperature(0), 
      printMaxOnly(false), reconnect(false), mode(MODE_IR), timeFormat(""),
      maxT(0)
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
  struct arg_str *a_degrees = arg_str0("g", "degrees", "K or C", "K- Kelvin, C- Celcius (default)");
  struct arg_str *a_mode = arg_str0("m", "mode", "i, a, t, f", "ir- infrared sensor, ambient- internal sensor, tick- counter value, file- read from the file");
  struct arg_lit *a_printMaxOnly = arg_lit0("1", NULL, "output max temperature only(no output every 1s when temperature growing)");
  struct arg_lit *a_reconnect = arg_lit0("r", "reconnect", "re-open COM port on error");
  struct arg_str *a_timeFormat = arg_str0("t", "timeformat", "<format>", "Output time format, e.g. \"" DEF_TIME_FORMAT "\". Default prints seconds since Unix epoch ");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_path, a_t0, a_t1, a_window,
    a_degrees, a_mode, a_printMaxOnly, a_reconnect, a_timeFormat,
    a_delay, a_verbosity, a_help, a_end 
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
       case 'K':
       case 'k':
          detectorOptions.degrees = DEG_K;
          break;
       default:
          break;
       }
     }
  }

  if (a_mode->count) {
     std::string d = *a_mode->sval;
     if (d.length() > 0) {
       // i, a, t, f
       switch (d[0])
       {
       case 'A':
       case 'a':
          detectorOptions.mode = MODE_AMBIENT;
          break;
       case 'T':
       case 't':
          detectorOptions.mode = MODE_TICK;
          break;
       case 'F':
       case 'f':
          detectorOptions.mode = MODE_FILE;
          break;
       default:
          break;
       }
     }
  }

  if (detectorOptions.path.empty()) {
    detectorOptions.path = DEF_DEVICE_PATH;
  }

  detectorOptions.printMaxOnly = a_printMaxOnly->count > 0;
  detectorOptions.reconnect = a_reconnect->count > 0;

  if (a_timeFormat->count) {
     detectorOptions.timeFormat = *a_timeFormat->sval;
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

  if (detectorOptions.dt < 0) {
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
  char c;
  switch (options.mode) {
  case MODE_AMBIENT:
    c = '1';
    break;
  case MODE_TICK:
    c = '2';
    break;
  case MODE_FILE:
    return 0;
  default:
    c = '0';
    break;
  }
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
    if (s[l - 1] == '\n') {
      size_t l2d;
      if (s[l - 2] == '\r') {
        l2d = l - 2;
      } else {
        l2d = l - 1;
      }
      s.erase(l2d);
      retval = atoi(s.c_str());
      return l;
    }
  }
  return 0;
}

static std::string getStartTimeStamp(
  const DetectorOptions &options
) {
  if (options.timeFormat.empty()) {
    std::stringstream ss;
    ss << std::dec << options.startTime;
    return ss.str();
  }

  return ltimeString(options.startTime, options.timeFormat);
}

static void putTemperature(
  DetectorOptions &options,
  int temperatureK100
) {
  switch (options.degrees) {
    case DEG_C:
        std::cout 
          << getStartTimeStamp(options) << "\t" 
          << std::fixed << std::setprecision(2)
          << (temperatureK100 - 27315) / 100.0  << std::endl;
      break;
    default:
        std::cout 
          << getStartTimeStamp(options) << "\t" 
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
    // just log data
    putTemperature(options, temperature);
    return;
  }

  // temperature is too low or too high. It is not a human.
  if (temperature < options.temperature0 || temperature > options.temperature1) {
    if (options.startTime > 0) {
      // Object is lost
      // Report about last object
      // options.maxT is correct value (because startTime > 0)
      putTemperature(options, options.maxT);
    } else {
      // Did not see any object yet
      return;
    }
    // reset time
    options.startTime = 0;
    options.sentTime = 0;
    options.sentTemperature = 0;
    // reset max
    options.maxT = 0;
    return;
  }

  // temperature is OK
  
  if (options.startTime == 0) {
    // New measurement, just started
    options.startTime = time(NULL);
    options.maxT = temperature;
    // Ready for new measurements
    return;
  }

  // Have old measurements

  if (temperature > options.maxT) {
    // Store new max temperature
    options.maxT = temperature;
  }

  if (options.printMaxOnly) {
    // output if object is lost only
    return;
  }

  if (options.sentTime == 0) {
    // not sent yet
    // Check is it time to report
    if (options.currentTime - options.startTime > options.dt) {
      // need to report first time
      putTemperature(options, options.maxT);
      // remember time of last report
      options.sentTime = options.currentTime;
      options.sentTemperature = options.maxT; // remember, check on flush
    }
  } else {
    // Sent one or more report(s) already
    if (options.currentTime - options.sentTime > options.dt) {
        // need to re-report if temperature is higher
        if (options.maxT > options.sentTemperature) {
          putTemperature(options, options.maxT);
          options.sentTemperature = options.maxT; // remember, check on flush
        }
      }
      // remember time of last report
      options.sentTime = options.currentTime;
  }
}

static void readDevice(
  DetectorOptions &options
)
{
	std::stringstream buf;
	int c = 0;
  options.startTime = 0;
  options.maxT = 0;
  sendRequestObjectTemperature(options);
	while (!options.stopped) {
		int r = read(options.fd, &c, 1);
    switch (r)
    {
    case -1:  // error
    case 0:   // EOF
      // lost everything
      buf.clear();
      // try to re-open file
      if (options.reconnect) {
        if (options.fd) {
          closeDevice(options.fd);
        }
        options.fd = openDevice(options.path);
        continue;
      } else {
        // exit gracefully
        options.stopped = true;
        break;
      }
      break;
    default:
      if (options.verbosity > 2) {
        std::cerr << (char) c;
      }
      break;
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
