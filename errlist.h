#define OK                 					0
#define ERR_CODE_COMMAND_LINE		      -500
#define ERR_CODE_GET_DEVICE			      -501
#define ERR_CODE_BAD_STATUS		          -502


#define ERR_COMMAND_LINE        "Wrong parameter(s)"
#define ERR_GET_DEVICE          "Error get device "
#define ERR_BAD_STATUS          "Bad status"

#define MSG_INTERRUPTED 		"Interrupted "
#define MSG_PG_CONNECTED        "Connected"
#define MSG_PG_CONNECTING       "Connecting..."
#define MSG_DAEMON_STARTED      "Start daemon "
#define MSG_DAEMON_STARTED_1    ". Check syslog."

const char *strerror_humandetector(int errcode);
