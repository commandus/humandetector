#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstring>
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif    

#include "argtable3/argtable3.h"
#include "errlist.h"

#define USE_PULSE_AUDIO 1

#ifdef USE_PULSE_AUDIO
#include <pulse/pulseaudio.h>
#endif

#include "wavemap.h"
#include "number2words.h"
#include "util-cmd.h"


const std::string progname = "say-number";
#define DEF_CONFIG_FILE_NAME ".say-number"
#define DEF_WAV_PATH    "wav"

class SayNumberOptions {
  public:
    bool stopped;
    std::string wavFilesPath;
    int precision;
    bool repeatadly;
    int verbosity;
    int t;

    SayNumberOptions() :
      stopped(false), wavFilesPath(""), precision(1), repeatadly(false), verbosity(0), t(0)
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
    SayNumberOptions &options,
	int argc,
	char* argv[]
)
{
  // device path
    struct arg_str *a_wav_files_path = arg_str0("w", "waves", "<path>", "Default none");
    struct arg_int *a_precision = arg_int0("p", "precision", "<0..6>", "Default 1");
    struct arg_lit *a_repeatadly = arg_lit0("R", "no-read", "do not read lines from stdin (say and exit)");
    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_int *a_t = arg_int0("t", "temperature", "<number>", NULL);
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_wav_files_path, a_precision, a_repeatadly,
        a_verbosity, a_t, a_help, a_end 
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

    if (a_wav_files_path->count) {
        options.wavFilesPath = *a_wav_files_path->sval;
    }

    if (a_precision->count) {
        options.precision = *a_precision->ival;
    }
    options.repeatadly = a_repeatadly->count == 0;
    options.verbosity = a_verbosity->count;

    if (options.precision < 0 || options.precision > 6) {
        std::cerr << "Precision must be in 0..6 ." << std::endl;
        nerrors++;
    }

    if (a_t->count)
        options.t = *a_t->ival;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Number pronunciation in Russian" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static SayNumberOptions options;

static void done()
{
 // destroy and free all
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

WaveMap wavemap;

#ifdef USE_PULSE_AUDIO
void context_state_cb(pa_context* context, void* mainloop);
void stream_state_cb(pa_stream *s, void *mainloop);
void stream_success_cb(pa_stream *stream, int success, void *userdata);
void stream_write_cb(pa_stream *stream, size_t requested_bytes, void *userdata);

int startPlay() {
    pa_threaded_mainloop *mainloop;
    pa_mainloop_api *mainloop_api;
    pa_context *context;
    pa_stream *stream;

    // Get a mainloop and its context
    mainloop = pa_threaded_mainloop_new();
    assert(mainloop);
    mainloop_api = pa_threaded_mainloop_get_api(mainloop);
    context = pa_context_new(mainloop_api, progname.c_str());
    assert(context);

    // Set a callback so we can wait for the context to be ready
    pa_context_set_state_callback(context, &context_state_cb, mainloop);

    // Lock the mainloop so that it does not run and crash before the context is ready
    pa_threaded_mainloop_lock(mainloop);

    // Start the mainloop
    assert(pa_threaded_mainloop_start(mainloop) == 0);
    assert(pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) == 0);

    // Wait for the context to be ready
    for(;;) {
        pa_context_state_t context_state = pa_context_get_state(context);
        assert(PA_CONTEXT_IS_GOOD(context_state));
        if (context_state == PA_CONTEXT_READY) break;
        pa_threaded_mainloop_wait(mainloop);
    }

    // Create a playback stream
    pa_sample_spec sample_specifications;
    sample_specifications.format = wavemap.header.get_pa_sample_format();
    sample_specifications.rate = wavemap.header.sampleRate;
    sample_specifications.channels = wavemap.header.numChannels;

    pa_channel_map map;
    if (wavemap.header.numChannels == 1)
        pa_channel_map_init_mono(&map);
    else
        pa_channel_map_init_stereo(&map);

    stream = pa_stream_new(context, progname.c_str(), &sample_specifications, &map);
    pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
    pa_stream_set_write_callback(stream, stream_write_cb, mainloop);

    // recommended settings, i.e. server uses sensible values
    pa_buffer_attr buffer_attr; 
    buffer_attr.maxlength = (uint32_t) -1;
    buffer_attr.tlength = (uint32_t) -1;
    buffer_attr.prebuf = (uint32_t) -1;
    buffer_attr.minreq = (uint32_t) -1;

    // Settings copied as per the chromium browser source
    pa_stream_flags_t stream_flags = static_cast<pa_stream_flags_t> (PA_STREAM_START_CORKED | PA_STREAM_INTERPOLATE_TIMING | 
        PA_STREAM_NOT_MONOTONIC | PA_STREAM_AUTO_TIMING_UPDATE |
        PA_STREAM_ADJUST_LATENCY);

    // Connect stream to the default audio output sink
    assert(pa_stream_connect_playback(stream, NULL, &buffer_attr, stream_flags, NULL, NULL) == 0);

    // Wait for the stream to be ready
    for(;;) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream);
        assert(PA_STREAM_IS_GOOD(stream_state));
        if (stream_state == PA_STREAM_READY)
            break;
        pa_threaded_mainloop_wait(mainloop);
    }

    pa_threaded_mainloop_unlock(mainloop);

    // Uncork the stream so it will start playing
    pa_stream_cork(stream, 0, stream_success_cb, mainloop);
}

void context_state_cb(pa_context* context, void* mainloop) {
    pa_threaded_mainloop_signal(static_cast<pa_threaded_mainloop*>(mainloop), 0);
}

void stream_state_cb(pa_stream *s, void *mainloop) {
    pa_threaded_mainloop_signal(static_cast<pa_threaded_mainloop*> (mainloop), 0);
}

void stream_write_cb(
	pa_stream *stream,
	size_t requested_bytes,
	void *userdata
) {
    int bytes_remaining = requested_bytes;
    while (bytes_remaining > 0) {
        uint8_t *buffer = NULL;
        size_t bytes_to_fill = 44100;
        size_t i;

        if (bytes_to_fill > bytes_remaining)
			bytes_to_fill = bytes_remaining;

        pa_stream_begin_write(stream, (void**) &buffer, &bytes_to_fill);
        wavemap.next(buffer, bytes_to_fill);
        pa_stream_write(stream, buffer, bytes_to_fill, NULL, 0LL, PA_SEEK_RELATIVE);
        bytes_remaining -= bytes_to_fill;
    }
}

void stream_success_cb(pa_stream *stream, int success, void *userdata) {
    return;
}
#endif

static int parseLine
(
    void** argtable,
    const std::string &line
)
{
    int t = 0;
    wordexp_t we;
    int argc;
    char ** argv = string2argv(&we, argc, line);
    int nerrors = arg_parse(argc, argv, argtable);
    if (((struct arg_int *) argtable[0])->count)
        t = *((struct arg_int *) argtable[0])->ival;
    argvFree(&we);
    return t;
}

void run(
    std::istream &strmin,
    std::ostream &strmout,
    SayNumberOptions &options
) {
    // thermometer measurement db connection options
    struct arg_int *a_t = arg_int0("t", "temperature", "<number>", NULL);
	struct arg_end *a_end = arg_end(20);
	void* argtable[] = { a_t, a_end };

    while (!options.stopped && strmin.good()) {
        std::string line;
        std::getline(strmin, line);
        if (strmin.eof())
            break;
        int t = parseLine(argtable, line);
        if (t)
            wavemap.say(t / 100., options.precision);        
        strmout << line << std::endl;  
        strmout.flush();
    }
}

int main(int argc, char *argv[]) {
    if (parseCmd(options, argc, argv) != 0) {
        exit(ERR_CODE_COMMAND_LINE);  
    };
  
#ifndef _MSC_VER
    setSignalHandler();
#endif
    WaveHeader hdr;
    if (options.wavFilesPath.empty())
        wavemap.loadResources();
    else
        wavemap.loadFiles(options.wavFilesPath + "/", ".wav");

    startPlay();

    if (options.t)
        wavemap.say(options.t / 100., options.precision);        

    if (options.repeatadly) {
        run(std::cin, std::cout, options);
    } else {
        while (true) {
#ifdef _MSC_VER
            Sleep(1);
#else
            usleep(100000);
#endif
            if (wavemap.nothingToSay()) {
#ifdef _MSC_VER
                Sleep(1);
#else
                sleep(1);
#endif
                break;
            }
        }
    }
}
