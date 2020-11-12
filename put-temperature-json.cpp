/**
 * @param gateid
 * @param time
 * @param temperature
 * @param tir
 * @param tmin
 * @param tambient
 * @return id
 * */
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <signal.h>

#include <curl/curl.h>

#include "argtable3/argtable3.h"

#include "errlist.h"
#include "utilstring.h"
#include "util-cmd.h"
#include "config-filename.h"

const std::string progname = "put-temperature-json";
#define DEF_CONFIG_FILE_NAME ".put-temperature-json"
#define DEF_TIMEOUT_STR "5"
#define DEF_TIMEOUT 5

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
	case SIGALRM:
		std::cerr << MSG_WS_TIMEOUT << std::endl;
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
	sigaction(SIGALRM, &action, NULL);
  // sigaction(SIGHUP, &action, NULL);
}
#endif

class PutJsonOptions {
private:
public:
  std::string url;
  uint64_t gateid;
  uint64_t secret;
  uint64_t userid;
  uint64_t id;
  time_t time;
  int t;
  int tir;
  int tmin;
  int tambient;
  bool stopped;
  bool repeatadly;
  bool waitStdin;
  int verbosity;
  int timeout;

  CURL *curl;

  PutJsonOptions() 
    : gateid(0), secret(0), userid(0), id(0), time(0), t(0), tir(0), tmin(0), tambient(0),
    url(""), stopped(false), repeatadly(false), verbosity(0), timeout(DEF_TIMEOUT), curl(NULL)
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
  PutJsonOptions &options,
	int argc,
	char* argv[]
)
{
  // thermemeter measurement db connection options
  struct arg_str *a_url = arg_str0("u", "url", "<address>", "e.g. https://acme.org/");
  struct arg_lit *a_repeatadly = arg_lit0("R", "no-read", "do not read lines from stdin");

  struct arg_int *a_gateid = arg_int0("g", "gate", "<number>", "Default 0");
  struct arg_int *a_secret = arg_int0("s", "secret", "<number>", "Default 0");
  struct arg_int *a_userid = arg_int0("c", "card", "<number>", "Default 0");
  struct arg_int *a_time = arg_int0(NULL, "time", "<number>", "Default now");
  struct arg_int *a_t = arg_int0("t", "temperature", "<number>", "Temperature in cC (100*C). Default 0");
  struct arg_int *a_tir = arg_int0(NULL, "tir", "<number>", "Temperature IR sensor in K*100 (100*C). Default 0");
  struct arg_int *a_tmin = arg_int0(NULL, "tmin", "<number>", "Min temperature in K*100 (100*C). Default 0");
  struct arg_int *a_tambient = arg_int0(NULL, "tambient", "<number>", "Case temperature  in K*100 (100*C). Default 0");

  struct arg_int *a_timeout = arg_int0(NULL, "timeout", "<number>", "Default " DEF_TIMEOUT_STR);

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
    a_url, a_repeatadly,
    a_gateid, a_secret, a_userid, a_time, a_t, a_tir, a_tmin, a_tambient,
    a_timeout,
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

  if (a_url->count) {
    options.url = *a_url->sval;
  }
  options.repeatadly = a_repeatadly->count == 0;
  
  if (a_gateid->count)
    options.gateid = *a_gateid->ival;
  if (a_secret->count)
    options.secret = *a_secret->ival;
  if (a_userid->count)
    options.userid = *a_userid->ival;
  if (a_time->count)
    options.time = *a_time->ival;
  if (a_t->count)
    options.t = *a_t->ival;
  if (a_tir->count)
    options.tir = *a_tir->ival;
  if (a_tmin->count)
    options.tmin = *a_tmin->ival;
  if (a_tambient->count)
    options.tambient = *a_tambient->ival;
  if (a_timeout->count)
    options.timeout = *a_timeout->ival;
  else
    options.timeout = DEF_TIMEOUT;

  options.verbosity = a_verbosity->count;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Put temperature measurement to the web service" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static void connectJson(
  PutJsonOptions &options
) {
  options.curl = curl_easy_init();
  if (options.curl) {
    curl_easy_setopt(options.curl, CURLOPT_URL, options.url.c_str());
    // enable TCP keep-alive for this transfer
    curl_easy_setopt(options.curl, CURLOPT_TCP_KEEPALIVE, 1L);
    // set keep-alive idle time to 120 seconds
    curl_easy_setopt(options.curl, CURLOPT_TCP_KEEPIDLE, 120L);
    // interval time between keep-alive probes: 60 seconds
    curl_easy_setopt(options.curl, CURLOPT_TCP_KEEPINTVL, 60L);
    // set maximum time the request is allowed to take
    curl_easy_setopt(options.curl, CURLOPT_TIMEOUT, options.timeout);
    // set timeout for the connect phase
    curl_easy_setopt(options.curl, CURLOPT_CONNECTTIMEOUT, options.timeout);
  }
}

static void disconnectJson(
  PutJsonOptions &options
) {
    if (options.curl) {
      curl_easy_cleanup(options.curl);
      options.curl = NULL;
  }
}

static void reconnectJson(
  PutJsonOptions &options
) {
}

static bool putMeasure(
  PutJsonOptions &options
) {
  if (options.curl) {
    std::stringstream ss;
    ss
      << "{\"gate\": "<< options.gateid
      << ",\"secret\": "<< options.secret
      << ",\"id\": "<< options.userid
      << ",\"time\": "<< options.time
      << ",\"t\": "<< options.t
      << ",\"tir\": "<< options.tir
      << ",\"tmin\": "<< options.tmin
      << ",\"tambient\": "<< options.tambient
      << "}";
    std::string s = ss.str();
    if (options.verbosity > 2) {
      std::cerr << s << std::endl;
    }
    // size of the POST data
    curl_easy_setopt(options.curl, CURLOPT_POSTFIELDSIZE, s.size());
    // pass in a pointer to the data - libcurl will not copy
    curl_easy_setopt(options.curl, CURLOPT_POSTFIELDS, s.c_str());
    CURLcode c = curl_easy_perform(options.curl);
    if (c != CURLE_OK) {
      std::cerr << "curl error " << c << ": " << curl_easy_strerror(c) << std::endl;
    }
  }
  return false;
}

static void parseLine
(
  PutJsonOptions &options,
  void* argtable[],
  const std::string &line
)
{
  wordexp_t we;
  int argc;

  char ** argv = string2argv(&we, argc, line);
	int nerrors = arg_parse(argc, argv, argtable);

  if (((struct arg_int *) argtable[0])->count)
    options.gateid = *((struct arg_int *) argtable[0])->ival;
  if (((struct arg_int *) argtable[1])->count)
    options.userid = *((struct arg_int *) argtable[1])->ival;
  if (((struct arg_str *) argtable[2])->count)
    options.time = strtoull(*((struct arg_str *) argtable[2])->sval, NULL, 10);
  if (((struct arg_int *) argtable[3])->count)
    options.t =  *((struct arg_int *) argtable[3])->ival;
  if (((struct arg_int *) argtable[4])->count)
    options.tir =  *((struct arg_int *) argtable[4])->ival;
  if (((struct arg_int *) argtable[5])->count)
    options.tmin =  *((struct arg_int *) argtable[5])->ival;
  if (((struct arg_int *) argtable[6])->count)
    options.tambient =  *((struct arg_int *) argtable[6])->ival;

  argvFree(&we);
}

static void run (
  std::istream &strmin,
  std::ostream &strmout,
  PutJsonOptions &options
) {
  // thermemeter measurement db connection options
  
  struct arg_int *a_gateid = arg_int0("g", "gate", "<number>", NULL);
  struct arg_int *a_userid = arg_int0("c", "card", "<number>", NULL);
  struct arg_str *a_time = arg_str0(NULL, "time", "<number>", NULL);
  struct arg_int *a_t = arg_int0("t", "temperature", "<number>", NULL);
  struct arg_int *a_tir = arg_int0(NULL, "tir", "<number>", NULL);
  struct arg_int *a_tmin = arg_int0(NULL, "tmin", "<number>", NULL);
  struct arg_int *a_tambient = arg_int0(NULL, "tambient", "<number>", NULL);
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { a_gateid, a_userid, a_time, a_t, a_tir, a_tmin, a_tambient, a_end };

  while (!options.stopped && strmin.good()) {
    std::string line;
    std::getline(strmin, line);

    if (strmin.eof())
      break;

    parseLine(options, argtable, line);
    bool sent = putMeasure(options);
    if (sent) {
      reconnectJson(options);
    }
    if (!sent) {
      strmout << line << " --senterror no-connection" << std::endl;  
    } else {
    }
    strmout.flush();
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}

PutJsonOptions options;

int main(
  int argc,
	char* argv[]
) {

  std::string config = file2string(getDefaultConfigFileName(DEF_CONFIG_FILE_NAME).c_str());
  parseConfigJson(
    options.url,
    options.gateid,
    options.secret,
    config
  );

  if (parseCmd(options, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };

#ifdef _MSC_VER
#else  
  setSignalHandler();
#endif

  connectJson(options);
  if (!options.curl) {
    std::cerr << "Error " << ERR_CODE_JSON_CONNECT << ": " << ERR_DB_CONNECT << std::endl;
    exit(ERR_CODE_DB_CONNECT);
  }
  if (options.repeatadly) {
    run(std::cin, std::cout, options);
  } else {
    uint64_t lastid = putMeasure(options);
    std::cout << lastid << std::endl;
  }
  disconnectJson(options);
  return OK;
}
