#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#define USE_PULSE_AUDIO 1

#ifdef USE_PULSE_AUDIO
#include <pulse/pulseaudio.h>
#endif

#include "wavemap.h"
#include "number2words.h"

#define FORMAT PA_SAMPLE_U8
#define RATE 44100

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
    context = pa_context_new(mainloop_api, "say-number");
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
    sample_specifications.format = FORMAT;
    sample_specifications.rate = RATE;
    sample_specifications.channels = 2;

    pa_channel_map map;
    pa_channel_map_init_stereo(&map);

    stream = pa_stream_new(context, "Playback numbers", &sample_specifications, &map);
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
        if (stream_state == PA_STREAM_READY) break;
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

int main(int argc, char *argv[]) {
    WaveHeader hdr;
    wavemap.loadFiles("wav/", ".wav");
    // std::cerr << wavemap.toString();
	
    // startPlay();
    
    // Play until we get a character
    // getc(stdin);

    /*
    for (double f = 31.1; f < 31.3; f += 0.1) {
        wavemap.say(f, 1);
    }
    */

    wavemap.say(35.1, 1);
    wavemap.say(32.2, 1);

//   wavemap.say(1, 0);
    /*
    std::cerr << "has sentence: " << wavemap.hasQueuedSentence() 
        << ", sentence: ";

    const SENTENCE *sentence(wavemap.getSentence());
    for (SENTENCE::const_iterator s(sentence->begin()); s != sentence->end(); s++) {
        std::cerr << std::setw(6) << *s << " ";
    }
    std::cerr << ", sentences: " << std::endl;
        
    const SENTENCES *sentences(wavemap.getSentences());
    for (SENTENCES::const_iterator ss(sentences->begin()); ss != sentences->end(); ss++) {
        for (SENTENCE::const_iterator s(ss->begin()); s != ss->end(); s++) {
            std::cerr << std::setw(6) << *s << " ";
        }
        std::cerr << std::endl;
    }
    */
    char buffer[1024];

    // calc size
    size_t sz = 0;
    size_t keysofs = 0;
    for (int i = 0; i <= 90; i++) {
        sz += wavemap.get(keysofs, &buffer, sizeof(buffer));
    }
    std::cerr << "size: " << sz 
        << ", header: " << sizeof(WaveHeader) 
        << ", total: " << sz + sizeof(WaveHeader) 
        << std::endl;

    // again
    keysofs = 0;
    /*
    for (double f = 31.1; f < 31.3; f += 0.1) {
        wavemap.say(f, 1);
    }
    */
   
   wavemap.say(35.1, 1);
   wavemap.say(32.2, 1);
   
//  wavemap.say(1, 0);

    std::ofstream f("out.wav", std::ios_base::binary | std::ios_base::out);
    wavemap.header.dataBytes = sz;
    wavemap.header.wavSize = sz + sizeof(WaveHeader) - 8;
    f.write((const char *) &wavemap.header, sizeof(WaveHeader));

    std::cerr << "----" << std::endl;
    for (int i = 0; i <= 90; i++) {
        size_t sz = wavemap.get(keysofs, &buffer, sizeof(buffer));
        f.write((const char *) &buffer, sz);
        std::cerr << "written bytes: " << sz << std::endl;
    }
    f.close();


    /*
    std::vector<int> keys;
    uint64_t numbers[] = { 18446744073709551615U };
    for (int i = 0; i < sizeof(numbers) / sizeof(uint64_t); i++) {
        keys.clear();
        std::cerr << numbers[i] << ":\t" << number2stringRU(numbers[i]) << std::endl;

        std::vector<int32_t> codes;
        number2codeRU(codes, numbers[i]);
        for (std::vector<int32_t>::const_iterator it(codes.begin()); it < codes.end(); ++it) {
            std::cerr << *it << std::endl;
        }
        std::cerr << std::endl;
    }
    */
}