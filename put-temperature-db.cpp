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
#include <ext/stdio_filebuf.h>
#include <postgresql/libpq-fe.h>
#include <arpa/inet.h>

#include "argtable3/argtable3.h"

#include "errlist.h"
#include "utilstring.h"
#include "util-cmd.h"
#include "config-filename.h"

/**
 * @see https://stackoverflow.com/questions/16375340/c-htonll-and-back
 */
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl((x) >> 32))

const std::string progname = "put-temperature-db";
#define DEF_CONFIG_FILE_NAME ".put-temperature-db"

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

class PutDbOptions {
private:
public:
  std::string conninfo;
  uint64_t gateid;
  time_t time;
  int t;
  int tir;
  int tmin;
  int tambient;
  bool stopped;
  bool repeatadly;

  PGconn *conn;
  ConnStatusType connStatus;

  PutDbOptions() 
    : gateid(0), time(0), t(0), tir(0), tmin(0), tambient(0),
    conninfo(""), stopped(false), repeatadly(false), conn(NULL)
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
  PutDbOptions &options,
	int argc,
	char* argv[]
)
{
  // thermemeter measurement db connection options
  struct arg_str *a_conninfo = arg_str0("d", "db", "<conninfo>", "e.g. host=.. dbname=.. user=.. password=..");
  struct arg_int *a_gateid = arg_int0("g", "gate", "<number>", "Default 0");
  struct arg_int *a_time = arg_int0(NULL, "time", "<number>", "Default now");
  struct arg_int *a_t = arg_int0("t", "temperature", "<number>", "Temperature in cC (100*C). Default 0");
  struct arg_int *a_tir = arg_int0(NULL, "tir", "<number>", "Temperature IR sensor in K*100 (100*C). Default 0");
  struct arg_int *a_tmin = arg_int0(NULL, "tmin", "<number>", "Min temperature in K*100 (100*C). Default 0");
  struct arg_int *a_tambient = arg_int0(NULL, "tambient", "<number>", "Case temperature  in K*100 (100*C). Default 0");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
    a_conninfo,
    a_gateid, a_time, a_t, a_tir, a_tmin, a_tambient,
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

  if (a_conninfo->count) {
    options.conninfo = *a_conninfo->sval;
  }

  if (a_gateid->count)
    options.gateid = *a_gateid->ival;
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

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Put temperature measurement to the PostgreSQL database" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static void connectDb(
  PutDbOptions &options
) {
  options.conn = PQconnectdb(options.conninfo.c_str());
  options.connStatus = PQstatus(options.conn);
}

static void disconnectDb(
  PutDbOptions &options
) {
  if (options.conn) {
    PQfinish(options.conn);
    options.conn = NULL;
  }
}

static void reconnectDb(
  PutDbOptions &options
) {
  disconnectDb(options);
  connectDb(options);
}

static uint64_t putMeasure(
  PutDbOptions &options
) {
  uint64_t ngateid = htonll(options.gateid);
  uint64_t ntime = htonll(options.time);
  int32_t nt = htonl(options.t);
  uint32_t ntir = htonl(options.tir);
  uint32_t ntambient = htonl(options.tambient);
  uint32_t ntmin = htonl(options.tmin);

  const char *values[6] = { 
    (const char *) &ngateid,
    (const char *) &ntime,
    (const char *) &nt,
    (const char *) &ntir,
    (const char *) &ntambient,
    (const char *) &ntmin
  };
  int lengths[6] = { (int) sizeof(ngateid), 
    (int) sizeof(ntime), (int) sizeof(nt), (int) sizeof(ntir),
    (int) sizeof(ntambient), (int) sizeof(ntmin)
  };
  int binary[6] = { 1, 1, 1, 1, 1, 1 };

  PGresult *r = PQexecParams(options.conn, "INSERT INTO measurement \
      (gateid, \"time\", t, tir, tambient, tmin) VALUES \
      ($1, $2, $3, $4, $5, $6) RETURNING id",
    6, NULL, values, lengths, binary, 0);

  uint64_t rr = 0;
  ExecStatusType status = PQresultStatus(r);
	if (status == PGRES_TUPLES_OK) {
    char *v = PQgetvalue(r, 0, 0);
    rr = strtoull(v, NULL, 10);
  } else {
    std::cerr << "Error " 
      << PQresStatus(status)
			<< ": " << PQresultErrorMessage(r)
      << std::endl;
  }
  PQclear(r);
  return rr;
}

static void parseLine
(
  PutDbOptions &options,
  void* argtable[],
  const std::string line
)
{
  wordexp_t *we = NULL;
  int argc;
  char ** argv = string2argv(&we, argc, line);
	int nerrors = arg_parse(argc, argv, argtable);

  if (((struct arg_int *) argtable[0])->count)
    options.gateid = *((struct arg_int *) argtable[0])->ival;
  if (((struct arg_int *) argtable[1])->count)
    options.time = *((struct arg_int *) argtable[1])->ival;
  if (((struct arg_int *) argtable[2])->count)
    options.t =  *((struct arg_int *) argtable[2])->ival;
  if (((struct arg_int *) argtable[3])->count)
    options.tir =  *((struct arg_int *) argtable[3])->ival;
  if (((struct arg_int *) argtable[4])->count)
    options.tmin =  *((struct arg_int *) argtable[4])->ival;
  if (((struct arg_int *) argtable[5])->count)
    options.tambient =  *((struct arg_int *) argtable[5])->ival;
  argvFree(we);
}

static void run (
  std::istream &strmin,
  std::ostream &strmout,
  PutDbOptions &options
) {
  // thermemeter measurement db connection options
  
  struct arg_int *a_gateid = arg_int0("g", "gate", "<number>", NULL);
  struct arg_int *a_time = arg_int0(NULL, "time", "<number>", NULL);
  struct arg_int *a_t = arg_int0("t", "temperature", "<number>", NULL);
  struct arg_int *a_tir = arg_int0(NULL, "tir", "<number>", NULL);
  struct arg_int *a_tmin = arg_int0(NULL, "tmin", "<number>", NULL);
  struct arg_int *a_tambient = arg_int0(NULL, "tambient", "<number>", NULL);
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { a_gateid, a_time, a_t, a_tir, a_tmin, a_tambient, a_end };

  while (!options.stopped && strmin.good()) {
    std::string line;
    std::getline(strmin, line);

    if (strmin.eof())
      break;

    parseLine(options, argtable, line);
    uint64_t id = putMeasure(options);
    if (!id) {
      reconnectDb(options);
      id = putMeasure(options);
    }
    if (!id) {
      strmout << line << " --dberror no-connection" << std::endl;  
    } else {
      strmout << line << " --id " << id << std::endl;
    }
    strmout.flush();
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}

PutDbOptions options;

int main(
  int argc,
	char* argv[]
) {

  std::string config = file2string(getDefaultConfigFileName(DEF_CONFIG_FILE_NAME).c_str());
  parseConfigDb(
    options.conninfo,
    config
  );

  if (parseCmd(options, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };

#ifdef _MSC_VER
#else  
  setSignalHandler();
#endif

  connectDb(options);
  if (options.connStatus == CONNECTION_BAD) {
    std::cerr << "Error " << ERR_CODE_DB_CONNECT << ": " << ERR_DB_CONNECT << std::endl;
    exit(ERR_CODE_DB_CONNECT);
  }
  if (options.repeatadly) {
    run(std::cin, std::cout, options);
  } else {
    uint64_t lastid = putMeasure(options);
    std::cout << lastid << std::endl;
  }
  disconnectDb(options);
  return OK;
}
