/*
 * Dependencies:
 * 	libmysqlclient.a, libmysqlclient-dev Debian package
 */
#include "wt-sigur.h"

#include "errlist.h"

#define DB_MAIN "tc-db-main"
#define DB_LOG "tc-db-log"
#define DB_PORT			3305

SigurOptions::SigurOptions() :
	connLogState(ERR_CODE_DB_NOTCONNECTED),
	host(""), user(""), passwd(""), dbLog(DB_LOG),
	area(0), direction(2), timeFormat("")
{
}

SigurOptions::SigurOptions(
	const std::string &dbLog
) :
	connLogState(ERR_CODE_DB_NOTCONNECTED),
	host(""), user(""), passwd(""), port(DB_PORT), dbLog(dbLog)
{
	
}

int SigurOptions::connect(
	const std::string &host,
	const std::string &user,
	const std::string &passwd,
	int port
)
{
	this->host = host;
	this->user = user;
	this->passwd = passwd;
	this->port = port;
	return reconnect();
}

int SigurOptions::reconnect() 
{
	connLogState = openMysqlConn(connLog, host.c_str(), user.c_str(), passwd.c_str(), dbLog.c_str(), port);
	return connLogState;
}

void SigurOptions::disconnect()
{
	if (connLogState != ERR_CODE_DB_NOTCONNECTED) {
		closeMysqlConn(connLog);
		connLogState = ERR_CODE_DB_NOTCONNECTED;
	}
}

SigurOptions::~SigurOptions()
{
	disconnect();
}

static const std::string DEF_QUERY_LOG_LAST_DIR_PREFIX("select UNIX_TIMESTAMP(logtime), CONV(SUBSTRING(hex(logdata), 23, 6), 16, 10), logtime from logs WHERE SUBSTRING(hex(logdata), 10, 1) = '");
static const std::string DEF_QUERY_LOG_LAST_DIR_AND("' and area = ");
static const std::string DEF_QUERY_LOG_LAST_DIR_SUFFIX(" order by id desc limit 1");

int SigurOptions::getLastSigurEmployee(
	int direction,
	int area,
	time_t &time
)
{
	this->direction = direction;
	this->area = area;
	return getLastSigurEmployee(time);
}

int SigurOptions::getLastSigurEmployee(
	time_t &time
)
{
	std::stringstream ss;
	ss << DEF_QUERY_LOG_LAST_DIR_PREFIX << direction 
		<< DEF_QUERY_LOG_LAST_DIR_AND << area
		<< DEF_QUERY_LOG_LAST_DIR_SUFFIX;
	if (mysql_query(&connLog, ss.str().c_str()) != 0) {
		return ERR_CODE_DB_EXEC_QUERY;
	}

	MYSQL_RES *res = mysql_store_result(&connLog);
	if (res == NULL) {
		return ERR_CODE_DB_GET_RESULT;
	}

	unsigned int rows = mysql_num_rows(res);
	unsigned int fields = mysql_num_fields(res);

	if (rows < 1 || fields < 2) {
		return ERR_CODE_DB_EMPTY;
	}
	MYSQL_ROW row = mysql_fetch_row(res);

	if (mysql_errno(&connLog) > 0) {
		return ERR_CODE_DB_FETCH_RESULT;
	}

	time = atol(row[0] ? row[0] : 0);
	int cardno = row[1] ? atoi(row[1]) : 0;
	
	mysql_free_result(res);

	return cardno;
}

int openMysqlConn(
	MYSQL &conn,
	const char *host,
	const char *user,
	const char *passwd,
	const char *db,
	int port

) {
  if (!mysql_init(&conn)) {
     return ERR_CODE_DB_CREATE_DESCRIPTOR;
  }
  if (!mysql_real_connect(&conn, host, user, passwd, db, port, NULL, 0)) {
     return ERR_CODE_DB_CONNECT;
  }
  return OK;
}

void closeMysqlConn(
	MYSQL &conn
) {
  mysql_close(&conn);
}
