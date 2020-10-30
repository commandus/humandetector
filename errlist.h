#define OK                 				0
#define ERR_CODE_COMMAND_LINE		    -500
#define ERR_CODE_OPEN_DEVICE		    -501
#define ERR_CODE_CLOSE_DEVICE		    -502
#define ERR_CODE_BAD_STATUS		        -503
#define ERR_CODE_DB_NOTCONNECTED        -504
#define ERR_CODE_DB_CREATE_DESCRIPTOR	-505
#define ERR_CODE_DB_CONNECT				-506
#define ERR_CODE_DB_EXEC_QUERY			-507
#define ERR_CODE_DB_GET_RESULT			-508
#define	ERR_CODE_DB_FETCH_RESULT		-509
#define ERR_CODE_DB_EMPTY				-510

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_OPEN_DEVICE         		"Error open device "
#define ERR_CLOSE_DEVICE        		"Error open device "
#define ERR_BAD_STATUS          		"Bad status"
#define ERR_DB_NOTCONNECTED     		"Database not connected"
#define ERR_DB_CREATE_DESCRIPTOR 		"Can't create MySQL descriptor"
#define ERR_DB_CONNECT					"Can't connect to MySQL server"
#define ERR_DB_EXEC_QUERY				"Can't execute SQL query"
#define ERR_DB_GET_RESULT				"Can't get the query result descriptor"
#define ERR_DB_FETCH_RESULT				"Can't fetch result"
#define ERR_DB_EMPTY					"Query return empty result"

#define MSG_INTERRUPTED 				"Interrupted "
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."

const char *strerror_humandetector(int errcode);
