#include <string>
#include <sstream>
#include <mysql/mysql.h>

class SigurOptions {
private:
	MYSQL connLog;
	int connLogState;
	std::string dbLog;
public:
	bool stopped;	// reserved
	std::string host;
	std::string user;
	std::string passwd;
	int port;
	int area;
	int direction;
	std::string timeFormat;

	SigurOptions();
	SigurOptions(
		const std::string &dbLog
	);
	~SigurOptions();
	
	int connect(
		const std::string &host,
		const std::string &user,
		const std::string &passwd,
		int port
	);
	int reconnect();

	void disconnect();

	/**
	 * direction: 1- out, 2- in
	 */
	int getLastSigurEmployee(
		int area,
		int direction,
		time_t &time
	);
	int getLastSigurEmployee(
		time_t &time
	);

};

int openMysqlConn(
	MYSQL &conn,
	const char *host,
	const char *user,
	const char *passwd,
	const char *db,
	int port
);

void closeMysqlConn(
	MYSQL &conn
);
