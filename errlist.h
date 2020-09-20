#define OK                 					0
#define ERR_CODE_COMMAND_LINE		      -500
#define ERR_CODE_OPEN_DEVICE		      -501
#define ERR_CODE_CLOSE_DEVICE		      -502
#define ERR_CODE_BAD_STATUS		          -503


#define ERR_COMMAND_LINE        "Wrong parameter(s)"
#define ERR_OPEN_DEVICE         "Error open device "
#define ERR_CLOSE_DEVICE        "Error open device "
#define ERR_BAD_STATUS          "Bad status"

#define MSG_INTERRUPTED 		"Interrupted "
#define MSG_PG_CONNECTED        "Connected"
#define MSG_PG_CONNECTING       "Connecting..."
#define MSG_DAEMON_STARTED      "Start daemon "
#define MSG_DAEMON_STARTED_1    ". Check syslog."

const char *strerror_humandetector(int errcode);
